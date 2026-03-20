const int ledPin = 2;

// 定义闪烁时序的状态数组
struct State {
  unsigned long duration; // 状态持续时间（毫秒）
  int ledState;           // LED 状态：HIGH = 亮，LOW = 灭
};

// SOS 时序：3短闪，3长闪，3短闪，最后长停顿（重复）
// 短闪亮 200ms，灭 200ms；长闪亮 600ms，灭 200ms；组间停顿 2000ms
State sequence[] = {
  // 第一个短闪组（S）
  {200, HIGH},  // 短亮1
  {200, LOW},   // 灭
  {200, HIGH},  // 短亮2
  {200, LOW},   // 灭
  {200, HIGH},  // 短亮3
  {200, LOW},   // 灭（准备进入长闪）

  // 长闪组（O）
  {600, HIGH},  // 长亮1
  {200, LOW},   // 灭
  {600, HIGH},  // 长亮2
  {200, LOW},   // 灭
  {600, HIGH},  // 长亮3
  {200, LOW},   // 灭（准备进入第二个短闪）

  // 第二个短闪组（S）
  {200, HIGH},  // 短亮1
  {200, LOW},   // 灭
  {200, HIGH},  // 短亮2
  {200, LOW},   // 灭
  {200, HIGH},  // 短亮3
  {2000, LOW}   // 长停顿（组间休息）
};

// 计算状态总数
const int numStates = sizeof(sequence) / sizeof(sequence[0]);

// 当前状态索引
int stateIndex = 0;

// 记录上一次状态切换的时刻（毫秒）
unsigned long previousMillis = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  // 将 LED 设置为第一个状态的初始值
  digitalWrite(ledPin, sequence[stateIndex].ledState);
  previousMillis = millis(); // 记录起始时间
}

void loop() {
  unsigned long currentMillis = millis();

  // 检查当前状态是否已经持续了足够的时间
  if (currentMillis - previousMillis >= sequence[stateIndex].duration) {
    // 切换到下一个状态
    stateIndex++;
    // 如果已到数组末尾，则回到开头（重新开始 SOS）
    if (stateIndex >= numStates) {
      stateIndex = 0;
    }

    // 更新 LED 状态
    digitalWrite(ledPin, sequence[stateIndex].ledState);
    // 记录本次切换的时间
    previousMillis = currentMillis;
  }

}