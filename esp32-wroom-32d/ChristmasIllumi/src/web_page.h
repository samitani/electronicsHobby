#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>Christmas Illumination</title>
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
            background: #0b0f19;
            color: #ffffff;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
            min-height: 100dvh;
            overflow-x: hidden;
            overflow-y: auto;
        }
        header {
            padding: 14px 16px;
            padding-top: calc(14px + env(safe-area-inset-top, 0px));
            background: linear-gradient(180deg, #161f30, #0f1624);
            display: flex;
            justify-content: space-between;
            align-items: center;
            border-bottom: 1px solid #23304a;
            box-shadow: 0 4px 12px rgba(0,0,0,0.4);
        }
        .header-title {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 1.15rem;
            font-weight: 700;
            color: #e74c3c;
            text-shadow: 0 0 10px rgba(231, 76, 60, 0.4);
        }
        .header-title span {
            color: #2ecc71;
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

        .container {
            padding: 16px;
            padding-bottom: calc(24px + env(safe-area-inset-bottom, 20px));
            display: flex;
            flex-direction: column;
            gap: 16px;
            max-width: 480px;
            margin: 0 auto;
            width: 100%;
        }

        /* 電源カード */
        .power-card {
            background: #141b2a;
            border: 1px solid #23304a;
            border-radius: 14px;
            padding: 14px 18px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .power-card .label {
            font-weight: 700;
            font-size: 1rem;
            color: #ecf0f1;
        }
        .power-btn {
            background: #ff4757;
            color: white;
            border: none;
            padding: 8px 24px;
            border-radius: 20px;
            font-size: 0.95rem;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.2s;
            box-shadow: 0 2px 8px rgba(255, 71, 87, 0.4);
        }
        .power-btn.on {
            background: #2ed573;
            box-shadow: 0 2px 8px rgba(46, 213, 115, 0.4);
        }

        /* セクションタイトル */
        .section-label {
            font-size: 0.75rem;
            font-weight: 700;
            letter-spacing: 1px;
            color: #7f8c8d;
            text-transform: uppercase;
            margin-bottom: -6px;
        }

        /* パターン選択グリッド */
        .pattern-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
        }
        .pattern-btn {
            background: #161e2e;
            border: 1px solid #283750;
            border-radius: 12px;
            padding: 14px 12px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 6px;
            color: #bdc3c7;
            cursor: pointer;
            transition: all 0.15s;
        }
        .pattern-btn .icon {
            font-size: 1.8rem;
        }
        .pattern-btn .name {
            font-size: 0.85rem;
            font-weight: 700;
        }
        .pattern-btn.active {
            background: linear-gradient(135deg, #c0392b, #8e44ad);
            color: #ffffff;
            border-color: #e74c3c;
            box-shadow: 0 4px 14px rgba(231, 76, 60, 0.4);
            transform: scale(1.02);
        }

        /* スライダーカード */
        .control-card {
            background: #141b2a;
            border: 1px solid #23304a;
            border-radius: 14px;
            padding: 14px 18px;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .card-header .label {
            font-size: 0.9rem;
            font-weight: 700;
            color: #ecf0f1;
        }
        .card-header .val {
            font-size: 0.85rem;
            font-weight: 700;
            color: #e74c3c;
        }
        input[type="range"] {
            -webkit-appearance: none;
            width: 100%;
            height: 6px;
            background: #23304a;
            border-radius: 3px;
            outline: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 22px;
            height: 22px;
            border-radius: 50%;
            background: #e74c3c;
            cursor: pointer;
            box-shadow: 0 0 8px rgba(231, 76, 60, 0.6);
        }

        /* オートサイクルスイッチ */
        .toggle-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .toggle-btn {
            background: #23304a;
            color: #bdc3c7;
            border: none;
            padding: 8px 18px;
            border-radius: 16px;
            font-size: 0.85rem;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.2s;
        }
        .toggle-btn.active {
            background: #3498db;
            color: white;
            box-shadow: 0 2px 8px rgba(52, 152, 219, 0.4);
        }
    </style>
</head>
<body>
    <header>
        <div class="header-title">🎄 Merry <span>Christmas</span></div>
        <div id="wsStatus" class="status-badge">Offline</div>
    </header>

    <div class="container">
        <!-- 電源 -->
        <div class="power-card">
            <div class="label">Power</div>
            <button id="powerBtn" class="power-btn on">ON</button>
        </div>

        <!-- パターン選択 -->
        <div class="section-label">Patterns</div>
        <div class="pattern-grid">
            <button class="pattern-btn active" data-id="0">
                <span class="icon">🍬</span>
                <span class="name">Candy Cane</span>
            </button>
            <button class="pattern-btn" data-id="1">
                <span class="icon">❄️</span>
                <span class="name">Snowfall</span>
            </button>
            <button class="pattern-btn" data-id="2">
                <span class="icon">🎄</span>
                <span class="name">Xmas Tree</span>
            </button>
            <button class="pattern-btn" data-id="3">
                <span class="icon">🌌</span>
                <span class="name">Aurora</span>
            </button>
            <button class="pattern-btn" data-id="4">
                <span class="icon">🔥</span>
                <span class="name">Fireplace</span>
            </button>
            <button class="pattern-btn" data-id="5">
                <span class="icon">🌈</span>
                <span class="name">Rainbow</span>
            </button>
        </div>

        <!-- オートサイクル -->
        <div class="control-card">
            <div class="toggle-row">
                <div class="label">Auto Cycle (20s)</div>
                <button id="autoCycleBtn" class="toggle-btn">OFF</button>
            </div>
        </div>

        <!-- 輝度調整 -->
        <div class="control-card">
            <div class="card-header">
                <div class="label">Brightness</div>
                <div id="brightVal" class="val">30%</div>
            </div>
            <input type="range" id="brightRange" min="10" max="255" value="77">
        </div>

        <!-- スピード調整 -->
        <div class="control-card">
            <div class="card-header">
                <div class="label">Speed</div>
                <div id="speedVal" class="val">1.0x</div>
            </div>
            <input type="range" id="speedRange" min="3" max="30" value="10">
        </div>
    </div>

    <script>
        let ws = null;
        let isPowerOn = true;
        let isAutoCycle = false;
        let currentPattern = 0;

        const wsStatus = document.getElementById('wsStatus');
        const powerBtn = document.getElementById('powerBtn');
        const autoCycleBtn = document.getElementById('autoCycleBtn');
        const brightRange = document.getElementById('brightRange');
        const brightVal = document.getElementById('brightVal');
        const speedRange = document.getElementById('speedRange');
        const speedVal = document.getElementById('speedVal');
        const patternBtns = document.querySelectorAll('.pattern-btn');

        function connectWebSocket() {
            const host = window.location.hostname || "192.168.1.100";
            ws = new WebSocket(`ws://${host}:81/`);

            ws.onopen = () => {
                wsStatus.textContent = "Online";
                wsStatus.classList.add("connected");
                sendCommand("GET_STATE");
            };

            ws.onclose = () => {
                wsStatus.textContent = "Offline";
                wsStatus.classList.remove("connected");
                setTimeout(connectWebSocket, 1500);
            };

            ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    if (data.type === "STATE") {
                        applyState(data);
                    }
                } catch(e) {
                    console.error("Parse error:", event.data);
                }
            };
        }

        function applyState(data) {
            isPowerOn = !!data.power;
            powerBtn.textContent = isPowerOn ? "ON" : "OFF";
            powerBtn.className = "power-btn" + (isPowerOn ? " on" : "");

            currentPattern = data.pattern;
            patternBtns.forEach(btn => {
                if (parseInt(btn.dataset.id) === currentPattern) {
                    btn.classList.add('active');
                } else {
                    btn.classList.remove('active');
                }
            });

            isAutoCycle = !!data.auto;
            autoCycleBtn.textContent = isAutoCycle ? "ON" : "OFF";
            autoCycleBtn.className = "toggle-btn" + (isAutoCycle ? " active" : "");

            brightRange.value = data.brightness;
            brightVal.textContent = Math.round((data.brightness / 255) * 100) + "%";

            speedRange.value = Math.round(data.speed * 10);
            speedVal.textContent = data.speed.toFixed(1) + "x";
        }

        function sendCommand(cmd) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(cmd);
            }
        }

        // 電源切り替え
        powerBtn.addEventListener('click', () => {
            isPowerOn = !isPowerOn;
            powerBtn.textContent = isPowerOn ? "ON" : "OFF";
            powerBtn.className = "power-btn" + (isPowerOn ? " on" : "");
            sendCommand("SET_POWER " + (isPowerOn ? "1" : "0"));
        });

        // パターン選択
        patternBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                const id = parseInt(btn.dataset.id);
                patternBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                currentPattern = id;
                sendCommand("SET_PATTERN " + id);
            });
        });

        // オートサイクル切り替え
        autoCycleBtn.addEventListener('click', () => {
            isAutoCycle = !isAutoCycle;
            autoCycleBtn.textContent = isAutoCycle ? "ON" : "OFF";
            autoCycleBtn.className = "toggle-btn" + (isAutoCycle ? " active" : "");
            sendCommand("SET_AUTO_CYCLE " + (isAutoCycle ? "1" : "0"));
        });

        // 輝度スライダー
        brightRange.addEventListener('input', () => {
            const val = parseInt(brightRange.value);
            brightVal.textContent = Math.round((val / 255) * 100) + "%";
            sendCommand("SET_BRIGHTNESS " + val);
        });

        // スピードスライダー
        speedRange.addEventListener('input', () => {
            const spd = (parseInt(speedRange.value) / 10).toFixed(1);
            speedVal.textContent = spd + "x";
            sendCommand("SET_SPEED " + spd);
        });

        window.addEventListener('DOMContentLoaded', connectWebSocket);
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
