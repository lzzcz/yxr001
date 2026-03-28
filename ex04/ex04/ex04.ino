#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 500
#define DEBOUNCE_DELAY 30

bool ledState = false;
bool lastTouchState = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
}

void loop() {
  int touchValue = touchRead(TOUCH_PIN);
  bool currentTouchState = (touchValue < THRESHOLD);

  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  // 检测上升沿（从未触摸变为触摸）
  if (currentTouchState == true && lastTouchState == false) {
    delay(DEBOUNCE_DELAY);           // 防抖延时
    // 延时后再次读取，确认仍然触摸
    if (touchRead(TOUCH_PIN) < THRESHOLD) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      Serial.print("LED Toggled -> ");
      Serial.println(ledState);
    }
  }

  lastTouchState = currentTouchState; // 更新上一次状态
  delay(50);
}