#include <WiFi.h>
#include <WebServer.h>

// ==================== 引脚定义 ====================
const int LED_PIN = 2;          // 板载LED (GPIO2)
const int TOUCH_PIN = 4;        // 触摸引脚 GPIO4 (对应触摸通道 T0)

// ==================== Wi-Fi AP 配置 ====================
const char* ssid = "ESP32_Security";
const char* password = "12345678";

// ==================== 全局状态变量 ====================
bool armed = false;          // 布防状态
bool alarmActive = false;    // 报警触发标志（LED闪烁锁定）
bool ledState = false;       // LED当前亮灭状态（用于闪烁）
unsigned long lastToggle = 0;
const unsigned long BLINK_INTERVAL = 100;  // 报警闪烁间隔 100ms

WebServer server(80);

// ==================== 辅助函数 ====================
void setLED(bool on) {
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  ledState = on;
}

void stopAlarm() {
  alarmActive = false;
  setLED(false);
  Serial.println("报警已停止");
}

void triggerAlarm() {
  if (!alarmActive) {
    alarmActive = true;
    Serial.println("⚠️ 触发报警！LED开始狂闪");
  }
}

void updateAlarmBlink() {
  if (!alarmActive) return;
  
  unsigned long now = millis();
  if (now - lastToggle >= BLINK_INTERVAL) {
    lastToggle = now;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}

void checkTouch() {
  if (!armed) return;
  if (alarmActive) return;
  
  int touchValue = touchRead(TOUCH_PIN);
  if (touchValue < 500) {
    triggerAlarm();
  }
}

// ==================== HTTP 请求处理 ====================
void handleArm() {
  armed = true;
  if (alarmActive) stopAlarm();
  Serial.println("系统布防");
  server.send(200, "text/plain", "Armed");
}

void handleDisarm() {
  armed = false;
  if (alarmActive) stopAlarm();
  Serial.println("系统撤防");
  server.send(200, "text/plain", "Disarmed");
}

void handleStatus() {
  String status;
  if (alarmActive) status = "alarm";
  else if (armed) status = "armed";
  else status = "disarmed";
  server.send(200, "application/json", "{\"status\":\"" + status + "\"}");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes">
    <title>ESP32 安防报警器</title>
    <style>
        * { box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(145deg, #1e2a2e 0%, #0f171a 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 0;
            padding: 20px;
        }
        .card {
            background: rgba(30, 40, 45, 0.85);
            backdrop-filter: blur(8px);
            border-radius: 48px;
            padding: 32px 28px;
            max-width: 500px;
            width: 100%;
            text-align: center;
            box-shadow: 0 20px 35px rgba(0,0,0,0.4);
            border: 1px solid rgba(255,255,255,0.15);
        }
        h1 {
            font-size: 2rem;
            margin: 0 0 8px;
            color: #f0f3f8;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 12px;
        }
        .shield-icon { font-size: 2rem; }
        .status-area {
            background: #00000055;
            border-radius: 60px;
            padding: 15px;
            margin: 25px 0;
        }
        .status-label {
            font-size: 0.85rem;
            letter-spacing: 1px;
            color: #aabfcb;
        }
        .status-value {
            font-size: 2rem;
            font-weight: bold;
            margin-top: 5px;
        }
        .armed { color: #4caf50; text-shadow: 0 0 5px #4caf50aa; }
        .disarmed { color: #f44336; }
        .alarm { color: #ff9800; animation: pulse 1s infinite; }
        @keyframes pulse {
            0% { opacity: 1; text-shadow: 0 0 0 #ff9800; }
            100% { opacity: 1; text-shadow: 0 0 12px #ff9800; }
        }
        .button-group {
            display: flex;
            gap: 20px;
            justify-content: center;
            margin: 30px 0 20px;
        }
        button {
            font-size: 1.2rem;
            padding: 12px 28px;
            border: none;
            border-radius: 60px;
            font-weight: bold;
            cursor: pointer;
            transition: 0.2s;
            background: #2c3e3f;
            color: white;
            box-shadow: 0 5px 10px rgba(0,0,0,0.2);
        }
        button:active { transform: scale(0.96); }
        .btn-arm { background: #2e7d32; }
        .btn-arm:hover { background: #1b5e20; }
        .btn-disarm { background: #c62828; }
        .btn-disarm:hover { background: #b71c1c; }
        .touch-note {
            font-size: 0.8rem;
            background: #00000055;
            border-radius: 30px;
            padding: 8px;
            margin-top: 20px;
            color: #bdd4e2;
        }
        footer {
            font-size: 0.7rem;
            margin-top: 25px;
            color: #6c8d9c;
        }
    </style>
</head>
<body>
<div class="card">
    <h1>
        <span class="shield-icon">🛡️</span> 安防报警器
    </h1>
    <div class="status-area">
        <div class="status-label">当前状态</div>
        <div class="status-value" id="statusText">--</div>
    </div>
    <div class="button-group">
        <button class="btn-arm" id="armBtn">🔒 布防</button>
        <button class="btn-disarm" id="disarmBtn">🔓 撤防</button>
    </div>
    <div class="touch-note">
        👆 触摸传感器 (GPIO4) — 布防后触碰立即触发报警，LED狂闪，必须撤防才能停止。
    </div>
    <footer>
        ⚡ 报警锁定 | 网页远程撤防
    </footer>
</div>

<script>
    const statusDiv = document.getElementById('statusText');
    const armBtn = document.getElementById('armBtn');
    const disarmBtn = document.getElementById('disarmBtn');

    function updateUI(status) {
        statusDiv.innerText = (status === 'armed') ? '🔐 布防中' :
                              (status === 'alarm') ? '🚨 报警中！' : '🔓 已撤防';
        statusDiv.className = 'status-value ' + status;
    }

    async function fetchStatus() {
        try {
            const response = await fetch('/status');
            const data = await response.json();
            updateUI(data.status);
        } catch(e) {
            console.error('状态获取失败', e);
        }
    }

    async function sendCommand(cmd) {
        try {
            const response = await fetch('/' + cmd);
            if (response.ok) fetchStatus();
        } catch(e) {
            console.error('命令发送失败', e);
        }
    }

    armBtn.addEventListener('click', () => sendCommand('arm'));
    disarmBtn.addEventListener('click', () => sendCommand('disarm'));

    fetchStatus();
    setInterval(fetchStatus, 1000);
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html; charset=UTF-8", html);
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n启动安防报警器模拟系统");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 触摸引脚 GPIO4 无需额外初始化，touchRead 即可

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP热点IP地址: ");
  Serial.println(myIP);
  Serial.println("请连接WiFi: " + String(ssid) + "  密码: " + String(password));
  Serial.print("浏览器访问: http://");
  Serial.println(myIP);

  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("HTTP服务器已启动");
  Serial.println("系统初始状态：撤防");
}

// ==================== 主循环 ====================
void loop() {
  server.handleClient();
  checkTouch();
  updateAlarmBlink();
  delay(10);
}
