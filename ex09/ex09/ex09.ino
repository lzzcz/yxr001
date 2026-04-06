#include <WiFi.h>
#include <WebServer.h>

// 引脚定义
const int TOUCH_PIN = 4;        // 触摸引脚 GPIO4 (T0)

// Wi-Fi AP配置
const char* ssid = "ESP32_Dashboard";
const char* password = "12345678";

WebServer server(80);

// 读取触摸传感器数值（0-4095）
int readTouchValue() {
  return touchRead(TOUCH_PIN);
}

// 处理AJAX请求：返回JSON格式的触摸值
void handleData() {
  int value = readTouchValue();
  String json = "{\"touch\":" + String(value) + "}";
  server.send(200, "application/json", json);
}

// 主页面：实时仪表盘（适配高数值范围）
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes">
    <title>ESP32 实时传感器仪表盘</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', 'Poppins', system-ui, sans-serif;
            background: linear-gradient(145deg, #0a0f1c 0%, #0b1525 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .dashboard {
            background: rgba(18, 25, 45, 0.65);
            backdrop-filter: blur(12px);
            border-radius: 64px;
            padding: 32px 28px 40px;
            width: 100%;
            max-width: 550px;
            text-align: center;
            box-shadow: 0 25px 45px rgba(0,0,0,0.4), inset 0 1px 1px rgba(255,255,255,0.1);
            border: 1px solid rgba(255,255,255,0.15);
        }
        h1 {
            font-size: 1.9rem;
            font-weight: 600;
            color: #eef5ff;
            letter-spacing: -0.5px;
            margin-bottom: 12px;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 12px;
        }
        .sensor-icon {
            font-size: 2rem;
        }
        .sub {
            color: #8aaec0;
            font-size: 0.85rem;
            margin-bottom: 35px;
            border-bottom: 1px solid #2a3a4a;
            display: inline-block;
            padding-bottom: 8px;
        }
        .value-container {
            background: #01050e;
            border-radius: 72px;
            padding: 30px 20px;
            margin: 15px 0 25px;
            box-shadow: inset 0 5px 10px rgba(0,0,0,0.5), 0 2px 4px rgba(255,255,255,0.05);
        }
        .sensor-value {
            font-size: 5.5rem;
            font-weight: 800;
            font-family: 'JetBrains Mono', 'Fira Code', monospace;
            color: #3bc9db;
            text-shadow: 0 0 10px #3bc9db88;
            letter-spacing: 4px;
        }
        .unit {
            font-size: 1.8rem;
            font-weight: 500;
            color: #7f9eb5;
            margin-left: 8px;
        }
        .indicator-bar {
            background: #1e2a36;
            border-radius: 40px;
            height: 12px;
            width: 100%;
            margin: 20px 0;
            overflow: hidden;
        }
        .bar-fill {
            width: 0%;
            height: 100%;
            background: linear-gradient(90deg, #3bc9db, #0e7c96);
            border-radius: 40px;
            transition: width 0.1s linear;
        }
        .status-text {
            font-size: 0.9rem;
            color: #bdd8f0;
            background: #00000055;
            display: inline-block;
            padding: 6px 18px;
            border-radius: 40px;
            margin-top: 10px;
        }
        .note {
            font-size: 0.7rem;
            color: #5f7f97;
            margin-top: 28px;
        }
        @keyframes pulse {
            0% { opacity: 0.7; }
            100% { opacity: 1; }
        }
        .live-badge {
            display: inline-block;
            width: 10px;
            height: 10px;
            background: #3bc9db;
            border-radius: 50%;
            margin-right: 6px;
            animation: pulse 1s infinite;
        }
        @media (max-width: 500px) {
            .sensor-value { font-size: 3.5rem; }
            .unit { font-size: 1.3rem; }
            .dashboard { padding: 24px 20px; }
        }
    </style>
</head>
<body>
<div class="dashboard">
    <h1>
        <span class="sensor-icon">🖐️</span> 触摸仪表盘
    </h1>
    <div class="sub">ESP32 实时传感器监控</div>

    <div class="value-container">
        <span class="sensor-value" id="touchValue">---</span>
        <span class="unit">Raw</span>
    </div>

    <div class="indicator-bar">
        <div class="bar-fill" id="barFill"></div>
    </div>

    <div class="status-text">
        <span class="live-badge"></span> 实时更新中 (100ms)
    </div>
    <div class="note">
        👆 手指靠近 GPIO4 引脚 → 数值减小（典型从1000降至200）<br>
        ✋ 离开后数值恢复
    </div>
</div>

<script>
    const valueSpan = document.getElementById('touchValue');
    const barFill = document.getElementById('barFill');

    // 根据实际硬件校准的映射参数
    // 未触摸时典型值 ~1000，强触摸时典型值 ~200
    const MAX_TOUCH = 1200;   // 无触摸时的参考上限（略高于1000）
    const MIN_TOUCH = 200;    // 强触摸时的参考下限（略低于200）

    function updateDisplay(rawValue) {
        valueSpan.innerText = rawValue;
        
        // 计算百分比：触摸值越低，百分比越高（进度条向右增长）
        let percent = (MAX_TOUCH - rawValue) / (MAX_TOUCH - MIN_TOUCH) * 100;
        // 限制范围 0% - 100%
        percent = Math.min(100, Math.max(0, percent));
        barFill.style.width = percent + '%';
        
        // 根据触摸强度改变进度条颜色：强触摸（值<300）时变为红色警告
        if (rawValue < 300) {
            barFill.style.background = "linear-gradient(90deg, #ff6b6b, #c92a2a)";
        } else if (rawValue < 600) {
            barFill.style.background = "linear-gradient(90deg, #ffa94d, #e8590c)";
        } else {
            barFill.style.background = "linear-gradient(90deg, #3bc9db, #0e7c96)";
        }
    }

    // 使用fetch定期获取最新触摸值
    function fetchTouchValue() {
        fetch('/data')
            .then(response => response.json())
            .then(data => {
                if (data.touch !== undefined) {
                    updateDisplay(data.touch);
                }
            })
            .catch(error => console.error('请求失败:', error));
    }

    // 每100毫秒拉取一次，实现流畅实时效果
    setInterval(fetchTouchValue, 100);
    // 立即执行一次避免空白
    fetchTouchValue();
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html; charset=UTF-8", html);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n启动实时传感器仪表盘（适配高数值范围）");

  // 触摸引脚无需初始化

  // 启动Wi-Fi AP模式
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP热点IP地址: ");
  Serial.println(myIP);
  Serial.println("请连接WiFi: " + String(ssid) + "  密码: " + String(password));
  Serial.print("打开浏览器访问: http://");
  Serial.println(myIP);

  // 配置路由
  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("HTTP服务器已启动");
}

void loop() {
  server.handleClient();
  delay(5);
}
