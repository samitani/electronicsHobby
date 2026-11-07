#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>LED Shoot</title>
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
            font-size: 1.05rem;
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

        /* モード選択タブ */
        .mode-tabs {
            display: flex;
            background: #141824;
            padding: 6px 12px;
            gap: 8px;
            border-bottom: 1px solid #2d3345;
        }
        .tab-btn {
            flex: 1;
            background: #1f2535;
            border: 1px solid #363e52;
            color: #8f9bb3;
            padding: 8px 4px;
            border-radius: 8px;
            font-size: 0.82rem;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.2s;
        }
        .tab-btn.active {
            background: linear-gradient(135deg, #00d2ff, #0077b6);
            color: #ffffff;
            border-color: #00d2ff;
            box-shadow: 0 2px 8px rgba(0, 210, 255, 0.35);
        }

        /* ダッシュボードエリア */
        .dashboard {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 8px;
            padding: 8px 14px;
            background: #161a24;
            border-bottom: 1px solid #2d3345;
        }
        .card {
            background: #202636;
            padding: 6px 8px;
            border-radius: 8px;
            text-align: center;
        }
        .card .label {
            font-size: 0.65rem;
            color: #8f9bb3;
            text-transform: uppercase;
            margin-bottom: 2px;
        }
        .card .value {
            font-size: 1.25rem;
            font-weight: 800;
            color: #ffffff;
        }
        .life-value {
            color: #ff4757 !important;
            letter-spacing: 2px;
            font-size: 1.05rem !important;
        }

        /* メイン通知・コントロールエリア */
        .center-stage {
            flex: 1;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            padding: 8px 12px;
            position: relative;
        }
        .game-status-text {
            font-size: 1.35rem;
            font-weight: 800;
            margin-bottom: 12px;
            text-align: center;
            color: #a4b0be;
            transition: all 0.2s;
        }
        .game-status-text.hit {
            color: #2ed573;
            transform: scale(1.08);
        }
        .game-status-text.miss {
            color: #ff4757;
            transform: scale(1.08);
        }
        .btn-action {
            background: linear-gradient(135deg, #00d2ff, #0056b3);
            color: white;
            border: none;
            padding: 12px 34px;
            border-radius: 30px;
            font-size: 1.15rem;
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
            padding: 12px 14px;
            padding-bottom: calc(16px + env(safe-area-inset-bottom, 20px));
            height: 34%;
            min-height: 150px;
            max-height: 240px;
            background: #161a24;
            border-top: 1px solid #2d3345;
        }
        .btn-color {
            flex: 1;
            border: none;
            border-radius: 16px;
            font-size: 1.35rem;
            font-weight: 900;
            color: white;
            cursor: pointer;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            box-shadow: 0 5px 0 rgba(0,0,0,0.4);
            transition: transform 0.05s, box-shadow 0.05s, filter 0.1s;
            outline: none;
        }
        .btn-color:active {
            transform: translateY(4px);
            box-shadow: 0 1px 0 rgba(0,0,0,0.4);
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
        <div id="pageTitle" class="title">LEDShoot : TIMING</div>
        <div id="wsStatus" class="status-badge">Offline</div>
    </header>

    <div class="mode-tabs">
        <button id="tabTiming" class="tab-btn active">⏱️ TIMING</button>
        <button id="tabShooting" class="tab-btn">🎯 SHOOTING</button>
    </div>

    <div class="dashboard">
        <div class="card">
            <div class="label">SCORE</div>
            <div id="scoreVal" class="value">0</div>
        </div>
        <div class="card">
            <div class="label">STREAK</div>
            <div id="streakVal" class="value">0</div>
        </div>
        <div class="card" id="lifeCard">
            <div id="lifeLabel" class="label">LIFE</div>
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
        let currentMode = "TIMING"; // "TIMING" or "SHOOTING"

        const wsStatus = document.getElementById('wsStatus');
        const pageTitle = document.getElementById('pageTitle');
        const tabTiming = document.getElementById('tabTiming');
        const tabShooting = document.getElementById('tabShooting');

        const scoreVal = document.getElementById('scoreVal');
        const streakVal = document.getElementById('streakVal');
        const lifeLabel = document.getElementById('lifeLabel');
        const lifeVal = document.getElementById('lifeVal');
        const statusText = document.getElementById('statusText');
        const actionBtn = document.getElementById('actionBtn');

        const btnRed = document.getElementById('btnRed');
        const btnGreen = document.getElementById('btnGreen');
        const btnBlue = document.getElementById('btnBlue');

        function setMode(mode) {
            currentMode = mode;
            if (mode === "TIMING") {
                tabTiming.classList.add('active');
                tabShooting.classList.remove('active');
                pageTitle.textContent = "LEDShoot : TIMING";
                lifeLabel.textContent = "LIFE";
                lifeVal.textContent = "❤️❤️❤️";
                statusText.textContent = isPlaying ? "WATCH THE TARGET!" : "TAP START TO PLAY";
            } else {
                tabShooting.classList.add('active');
                tabTiming.classList.remove('active');
                pageTitle.textContent = "LEDShoot : SHOOTING";
                lifeLabel.textContent = "CONDITION";
                lifeVal.textContent = "1HIT KO💀";
                statusText.textContent = isPlaying ? "SHOOT SAME COLOR!" : "TAP START TO PLAY";
            }
            scoreVal.textContent = "0";
            streakVal.textContent = "0";
            sendCommand("SET_MODE " + mode);
        }

        tabTiming.addEventListener('click', () => setMode("TIMING"));
        tabShooting.addEventListener('click', () => setMode("SHOOTING"));

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
                        statusText.textContent = (currentMode === "TIMING") ? "WATCH THE TARGET!" : "FIRE AT ENEMIES!";
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
                    if (currentMode === "TIMING") {
                        let hearts = "";
                        for (let i = 0; i < msg.val; i++) hearts += "❤️";
                        for (let i = msg.val; i < 3; i++) hearts += "🖤";
                        lifeVal.textContent = hearts;
                    }
                    break;
                case "HIT":
                    statusText.textContent = "HIT! +" + msg.val;
                    statusText.className = "game-status-text hit";
                    if (navigator.vibrate) navigator.vibrate(35);
                    break;
                case "MISS":
                    statusText.textContent = "MISS!";
                    statusText.className = "game-status-text miss";
                    if (navigator.vibrate) navigator.vibrate(100);
                    break;
                case "GAME_OVER":
                    statusText.textContent = `FINAL: ${msg.val} (MAX: ${msg.v2})`;
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

        window.addEventListener('DOMContentLoaded', connectWebSocket);
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
