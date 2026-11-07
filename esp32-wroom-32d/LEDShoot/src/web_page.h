#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>LED Shoot - Timing Game</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            user-select: none;
            -webkit-user-select: none;
            touch-action: manipulation;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background: #11141a;
            color: #ffffff;
            display: flex;
            flex-direction: column;
            height: 100vh;
            height: 100dvh;
            width: 100vw;
            overflow: hidden;
            justify-content: space-between;
        }
        header {
            padding: 12px 16px;
            padding-top: calc(12px + env(safe-area-inset-top, 0px));
            background: #1a1e29;
            display: flex;
            justify-content: space-between;
            align-items: center;
            border-bottom: 1px solid #2d3345;
        }
        .title {
            font-size: 1.1rem;
            font-weight: 700;
            letter-spacing: 0.5px;
            color: #00d2ff;
        }
        .status-badge {
            font-size: 0.75rem;
            padding: 4px 8px;
            border-radius: 12px;
            background: #ff4757;
            color: white;
            font-weight: 600;
            transition: background 0.3s;
        }
        .status-badge.connected {
            background: #2ed573;
        }

        /* ダッシュボードエリア */
        .dashboard {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 8px;
            padding: 10px 16px;
            background: #161a24;
            border-bottom: 1px solid #2d3345;
        }
        .card {
            background: #202636;
            padding: 8px 10px;
            border-radius: 8px;
            text-align: center;
        }
        .card .label {
            font-size: 0.7rem;
            color: #8f9bb3;
            text-transform: uppercase;
            margin-bottom: 4px;
        }
        .card .value {
            font-size: 1.3rem;
            font-weight: 800;
            color: #ffffff;
        }
        .life-value {
            color: #ff4757 !important;
            letter-spacing: 2px;
        }

        /* メイン通知・コントロールエリア */
        .center-stage {
            flex: 1;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            padding: 12px;
            position: relative;
        }
        .game-status-text {
            font-size: 1.5rem;
            font-weight: 800;
            margin-bottom: 12px;
            text-align: center;
            color: #a4b0be;
            transition: all 0.2s;
        }
        .game-status-text.hit {
            color: #2ed573;
            transform: scale(1.1);
        }
        .game-status-text.miss {
            color: #ff4757;
            transform: scale(1.1);
        }
        .btn-action {
            background: linear-gradient(135deg, #00d2ff, #0056b3);
            color: white;
            border: none;
            padding: 14px 36px;
            border-radius: 30px;
            font-size: 1.2rem;
            font-weight: 700;
            box-shadow: 0 4px 15px rgba(0, 210, 255, 0.4);
            cursor: pointer;
            transition: transform 0.1s, opacity 0.2s;
        }
        .btn-action:active {
            transform: scale(0.96);
            opacity: 0.9;
        }

        /* 3色操作ボタンエリア (iOSセーフエリア対応) */
        .controls {
            display: flex;
            gap: 12px;
            padding: 14px 16px;
            padding-bottom: calc(18px + env(safe-area-inset-bottom, 20px));
            height: 35%;
            min-height: 160px;
            max-height: 240px;
            background: #161a24;
            border-top: 1px solid #2d3345;
        }
        .btn-color {
            flex: 1;
            border: none;
            border-radius: 16px;
            font-size: 1.4rem;
            font-weight: 900;
            color: white;
            cursor: pointer;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            box-shadow: 0 6px 0 rgba(0,0,0,0.4);
            transition: transform 0.05s, box-shadow 0.05s, filter 0.1s;
            outline: none;
        }
        .btn-color:active {
            transform: translateY(4px);
            box-shadow: 0 2px 0 rgba(0,0,0,0.4);
            filter: brightness(1.2);
        }
        .btn-red {
            background: linear-gradient(180deg, #ff4757, #c0392b);
        }
        .btn-green {
            background: linear-gradient(180deg, #2ed573, #218c53);
        }
        .btn-blue {
            background: linear-gradient(180deg, #1e90ff, #0984e3);
        }
    </style>
</head>
<body>
    <header>
        <div class="title">LEDShoot : TIMING GAME</div>
        <div id="wsStatus" class="status-badge">Offline</div>
    </header>

    <div class="dashboard">
        <div class="card">
            <div class="label">SCORE</div>
            <div id="scoreVal" class="value">0</div>
        </div>
        <div class="card">
            <div class="label">STREAK</div>
            <div id="streakVal" class="value">0</div>
        </div>
        <div class="card">
            <div class="label">LIFE</div>
            <div id="lifeVal" class="value life-value">❤️❤️❤️</div>
        </div>
    </div>

    <div class="center-stage">
        <div id="statusText" class="game-status-text">TAP START TO PLAY</div>
        <button id="actionBtn" class="btn-action">START</button>
    </div>

    <div class="controls">
        <button id="btnRed" class="btn-color btn-red">RED</button>
        <button id="btnGreen" class="btn-color btn-green">GREEN</button>
        <button id="btnBlue" class="btn-color btn-blue">BLUE</button>
    </div>

    <script>
        let ws = null;
        let isPlaying = false;

        const wsStatus = document.getElementById('wsStatus');
        const scoreVal = document.getElementById('scoreVal');
        const streakVal = document.getElementById('streakVal');
        const lifeVal = document.getElementById('lifeVal');
        const statusText = document.getElementById('statusText');
        const actionBtn = document.getElementById('actionBtn');

        const btnRed = document.getElementById('btnRed');
        const btnGreen = document.getElementById('btnGreen');
        const btnBlue = document.getElementById('btnBlue');

        function connectWebSocket() {
            const host = window.location.hostname || "192.168.1.100";
            const wsUrl = `ws://${host}:81/`;
            ws = new WebSocket(wsUrl);

            ws.onopen = () => {
                wsStatus.textContent = "Online";
                wsStatus.classList.add("connected");
            };

            ws.onclose = () => {
                wsStatus.textContent = "Offline";
                wsStatus.classList.remove("connected");
                setTimeout(connectWebSocket, 1500);
            };

            ws.onerror = (err) => {
                ws.close();
            };

            ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    handleMessage(data);
                } catch(e) {
                    console.error("Parse error:", event.data);
                }
            };
        }

        function handleMessage(msg) {
            switch(msg.type) {
                case "GAME_STATE":
                    // 1: PLAYING, 2: GAMEOVER, 0: IDLE
                    if (msg.val === 1) {
                        isPlaying = true;
                        actionBtn.style.display = "none";
                        statusText.textContent = "WATCH THE LED!";
                        statusText.className = "game-status-text";
                    } else if (msg.val === 2) {
                        isPlaying = false;
                        actionBtn.textContent = "RESTART";
                        actionBtn.style.display = "block";
                        statusText.textContent = "GAME OVER";
                        statusText.className = "game-status-text miss";
                    } else {
                        isPlaying = false;
                        actionBtn.textContent = "START";
                        actionBtn.style.display = "block";
                        statusText.textContent = "READY";
                        statusText.className = "game-status-text";
                    }
                    break;
                case "SCORE_UPDATE":
                    scoreVal.textContent = msg.val;
                    break;
                case "STREAK_UPDATE":
                    streakVal.textContent = msg.val;
                    break;
                case "LIFE_UPDATE":
                    let hearts = "";
                    for (let i = 0; i < msg.val; i++) hearts += "❤️";
                    for (let i = msg.val; i < 3; i++) hearts += "🖤";
                    lifeVal.textContent = hearts;
                    break;
                case "HIT":
                    statusText.textContent = "HIT! +" + msg.val;
                    statusText.className = "game-status-text hit";
                    if (navigator.vibrate) navigator.vibrate(40);
                    break;
                case "MISS":
                    statusText.textContent = "MISS!";
                    statusText.className = "game-status-text miss";
                    if (navigator.vibrate) navigator.vibrate(120);
                    break;
                case "GAME_OVER":
                    statusText.textContent = `FINAL SCORE: ${msg.val} (MAX: ${msg.v2})`;
                    statusText.className = "game-status-text miss";
                    actionBtn.textContent = "RESTART";
                    actionBtn.style.display = "block";
                    break;
            }
        }

        function sendCommand(cmd) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(cmd);
            }
        }

        function bindButton(btn, cmd) {
            const trigger = (e) => {
                e.preventDefault();
                sendCommand(cmd);
            };
            btn.addEventListener('touchstart', trigger, { passive: false });
            btn.addEventListener('mousedown', trigger);
        }

        bindButton(btnRed, "PRESS_RED");
        bindButton(btnGreen, "PRESS_GREEN");
        bindButton(btnBlue, "PRESS_BLUE");

        actionBtn.addEventListener('click', () => {
            if (actionBtn.textContent === "START") {
                sendCommand("START");
            } else {
                sendCommand("RESTART");
            }
        });

        // 起動時WebSocket接続開始
        window.addEventListener('DOMContentLoaded', connectWebSocket);
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
