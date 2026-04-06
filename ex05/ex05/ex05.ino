#include <Arduino.h>

#define TOUCH_PIN   4
#define LED_PIN     2
#define TOUCH_THRESHOLD 800      
#define DEBOUNCE_DELAY 50        // 防抖延时（毫秒）

#define SPEED_LEVELS 3
const unsigned long stepDelays[SPEED_LEVELS] = {20, 10, 2};  // 差异明显的档位

byte currentLevel = 0;
unsigned long lastStepTime = 0;
int brightness = 0;
bool increasing = true;

// 触摸状态变量
bool lastTouchState = false;          // 上一次的触摸状态（用于上升沿检测）
unsigned long lastTouchTime = 0;      // 上次触发切换的时间（用于防抖）

void setup() {
  Serial.begin(115200);
  delay(1000);

  ledcAttach(LED_PIN, 5000, 8);
  ledcWrite(LED_PIN, 0);

  Serial.println("系统启动，触摸切换呼吸速度档位（1-3档）");
  Serial.println("当前档位: 1 (最慢)");
}

void loop() {
  int touchValue = touchRead(TOUCH_PIN);
  bool currentTouch = (touchValue < TOUCH_THRESHOLD);

  // ========== 触摸切换逻辑（上升沿检测 + 防抖） ==========
  // 如果当前触摸，且之前未触摸，并且距离上次切换已超过防抖时间
  if (currentTouch && !lastTouchState && (millis() - lastTouchTime > DEBOUNCE_DELAY)) {
    lastTouchTime = millis();          // 记录本次切换时间
    currentLevel = (currentLevel + 1) % SPEED_LEVELS;
    Serial.print(">>> 速度档位切换至: ");
    Serial.println(currentLevel + 1);
  }
  // 更新上一次触摸状态（用于下次检测）
  lastTouchState = currentTouch;

  // ========== 呼吸灯控制（非阻塞） ==========
  unsigned long now = millis();
  unsigned long stepDelay = stepDelays[currentLevel];

  if (now - lastStepTime >= stepDelay) {
    lastStepTime = now;

    if (increasing) {
      brightness++;
      if (brightness >= 255) {
        brightness = 255;
        increasing = false;
      }
    } else {
      brightness--;
      if (brightness <= 0) {
        brightness = 0;
        increasing = true;
      }
    }

    ledcWrite(LED_PIN, brightness);
  }
}