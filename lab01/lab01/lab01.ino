// 定义板载LED引脚，避免使用"魔法数字"
#define LED_PIN 2
#define LED_PIN_GREEN 4

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  // 初始化板载LED引脚为输出模式
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN_GREEN, OUTPUT); 
}

void loop() {
  Serial.println("Hello ESP32!");
  digitalWrite(LED_PIN, HIGH);  
    digitalWrite(REDGREEN, HIGH); // 点亮LED
  delay(1000);                   // 等待1秒
  digitalWrite(LED_PIN, LOW);
   digitalWrite(LED_PIN_GREEN, LOW);    // 熄灭LED
  delay(1000);              // 等待1秒
}
