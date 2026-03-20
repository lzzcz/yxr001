const int ledPin = 2;

// 记录上一次 LED 状态改变的时间（毫秒）
unsigned long previousMillis = 0;

// 闪烁间隔（亮 500ms，灭 500ms，周期 1Hz）
const long interval = 500;

// 当前 LED 状态
int ledState = LOW;

void setup() {
  // 初始化 LED 引脚为输出模式
  pinMode(ledPin, OUTPUT);
  // 设置初始状态为灭
  digitalWrite(ledPin, ledState);
}

void loop() {
  // 获取当前运行时间（毫秒）
  unsigned long currentMillis = millis();

  // 检查是否达到 500ms 间隔
  if (currentMillis - previousMillis >= interval) {
    // 保存本次状态改变的时间点
    previousMillis = currentMillis;

    // 翻转 LED 状态
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(ledPin, ledState);
  }

}