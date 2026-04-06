#include <WiFi.h>
#include <WebServer.h>

// LED引脚配置（多数ESP32开发板的板载LED为GPIO2）
const int LED_PIN = 2;

// PWM配置：频率5kHz，8位分辨率（0-255）
const int PWM_FREQ = 5000;
const int PWM_RES = 8;

// Wi-Fi AP模式配置（手机/电脑可直接连接，无需路由器）
const char* ssid = "ESP32_Dimmer";
const char* password = "12345678";

WebServer server(80);

// 记录当前亮度值，用于页面同步
int currentBrightness = 128;

// 设置LED亮度
void setLEDBrightness(int brightness) {
  brightness = constrain(brightness, 0, 255);
  ledcWrite(LED_PIN, brightness);   // 新版ESP32 API直接通过引脚写入PWM
  currentBrightness = brightness;
  Serial.print("亮度已设置为: ");
  Serial.println(brightness);
}

// 处理GET请求：/set?duty=数值
void handleSet() {
  if (server.hasArg("duty")) {
    int duty = server.arg("duty").toInt();
    setLEDBrightness(duty);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing parameter: duty");
  }
}

// 处理GET请求：/getDuty → 返回当前亮度，用于页面初始化同步
void handleGetDuty() {
  server.send(200, "text/plain", String(currentBrightness));
}

// 主页面：包含滑动条的HTML/CSS/JS
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes">
    <title>ESP32 无极调光器</title>
    <style>
        * {
            box-sizing: border-box;
            user-select: none;
        }
        body {
            font-family: system-ui, -apple-system, 'Segoe UI', Roboto, 'Helvetica Neue', sans-serif;
            background: linear-gradient(145deg, #1a2a3a 0%, #0f1a24 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 0;
            padding: 20px;
        }
        .card {
            background: rgba(30, 40, 50, 0.75);
            backdrop-filter: blur(8px);
            border-radius: 56px;
            padding: 32px 28px 42px;
            box-shadow: 0 25px 45px rgba(0, 0, 0, 0.3), inset 0 1px 1px rgba(255, 255, 255, 0.1);
            width: 100%;
            max-width: 520px;
            text-align: center;
            border: 1px solid rgba(255, 255, 255, 0.2);
        }
        h1 {
            font-size: 1.9rem;
            font-weight: 600;
            margin: 0 0 8px 0;
            color: #f0f3f8;
            letter-spacing: -0.3px;
            text-shadow: 0 2px 5px rgba(0,0,0,0.2);
        }
        .sub {
            color: #9aaebf;
            font-size: 0.85rem;
            margin-bottom: 32px;
            border-bottom: 1px dashed #3e5568;
            display: inline-block;
            padding-bottom: 6px;
        }
        .brightness-preview {
            background: #00000044;
            border-radius: 100px;
            padding: 12px 20px;
            margin: 20px 0 28px;
            display: inline-flex;
            align-items: baseline;
            gap: 12px;
            backdrop-filter: blur(4px);
        }
        .brightness-value {
            font-size: 3.2rem;
            font-weight: 700;
            color: #ffd966;
            background: #1e2a32;
            padding: 0 16px;
            border-radius: 60px;
            font-family: monospace;
            letter-spacing: 2px;
            box-shadow: inset 0 1px 3px #00000055, 0 1px 0 #4e6b7c;
        }
        .brightness-unit {
            font-size: 1.1rem;
            color: #c0d0dd;
            font-weight: 500;
        }
        .slider-container {
            margin: 35px 0 20px;
        }
        input[type=range] {
            -webkit-appearance: none;
            width: 100%;
            height: 8px;
            background: linear-gradient(90deg, #2c3e2f, #f5b042);
            border-radius: 10px;
            outline: none;
            cursor: pointer;
            box-shadow: inset 0 1px 2px #00000033, 0 1px 2px #ffffff1a;
        }
        input[type=range]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 28px;
            height: 28px;
            background: #ffdd99;
            border-radius: 50%;
            border: 2px solid #ffffff;
            box-shadow: 0 4px 12px rgba(0,0,0,0.3);
            transition: 0.1s;
            cursor: pointer;
        }
        input[type=range]::-webkit-slider-thumb:hover {
            transform: scale(1.2);
            background: #ffefbf;
        }
        .led-icon {
            font-size: 3rem;
            filter: drop-shadow(0 4px 6px #00000055);
            margin-bottom: 8px;
        }
        .status-text {
            font-size: 0.8rem;
            margin-top: 24px;
            color: #a2bbcf;
            background: #00000033;
            padding: 6px 14px;
            border-radius: 40px;
            display: inline-block;
        }
        footer {
            font-size: 0.7rem;
            margin-top: 28px;
            color: #5f7f97;
        }
        @media (max-width: 480px) {
            .card { padding: 24px 20px 32px; }
            .brightness-value { font-size: 2.5rem; padding: 0 12px; }
            h1 { font-size: 1.6rem; }
        }
    </style>
</head>
<body>
<div class="card">
    <div class="led-icon">💡</div>
    <h1>无极调光器</h1>
    <div class="sub">ESP32 PWM 平滑控制</div>

    <div class="brightness-preview">
        <span class="brightness-value" id="brightnessDisplay">128</span>
        <span class="brightness-unit">/ 255</span>
    </div>

    <div class="slider-container">
        <input type="range" id="brightnessSlider" min="0" max="255" value="128" step="1">
    </div>

    <div class="status-text">
        🎚️ 拖动滑块 · 实时调节亮度
    </div>
    <footer>
        ⚡ 异步请求 | 平滑PWM调光
    </footer>
</div>

<script>
    const slider = document.getElementById('brightnessSlider');
    const displaySpan = document.getElementById('brightnessDisplay');

    // 更新页面上的数值显示
    function updateDisplay(value) {
        displaySpan.innerText = value;
    }

    // 通过fetch发送亮度值到ESP32
    function sendBrightness(value) {
        fetch('/set?duty=' + value)
            .then(response => {
                if (!response.ok) console.warn('服务器响应异常: ', response.status);
            })
            .catch(error => console.error('请求失败，请检查网络连接: ', error));
    }

    // 监听滑动条实时变动（每次滑动都发送请求，实现平滑跟随）
    slider.addEventListener('input', function(e) {
        const val = e.target.value;
        updateDisplay(val);
        sendBrightness(val);
    });

    // 页面加载时从ESP32获取当前亮度，保持同步
    window.addEventListener('load', () => {
        fetch('/getDuty')
            .then(res => res.text())
            .then(val => {
                let duty = parseInt(val);
                if (!isNaN(duty) && duty >= 0 && duty <= 255) {
                    slider.value = duty;
                    updateDisplay(duty);
                }
            })
            .catch(err => console.log("获取初始亮度失败，使用默认值"));
    });
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html; charset=UTF-8", html);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n启动ESP32无极调光器");

  // 配置PWM：将LED引脚绑定到内部PWM通道（新版ESP32 API）
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);
  setLEDBrightness(128);   // 初始亮度中等

  // 启动Wi-Fi AP模式
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP热点IP地址: ");
  Serial.println(myIP);
  Serial.println("请用手机/电脑连接WiFi: " + String(ssid) + "  密码: " + String(password));
  Serial.print("打开浏览器访问: http://");
  Serial.println(myIP);

  // 配置HTTP路由
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/getDuty", handleGetDuty);

  server.begin();
  Serial.println("HTTP服务器已启动");
}

void loop() {
  server.handleClient();   // 持续处理客户端请求
  delay(2);                // 微小延时，确保系统稳定
}