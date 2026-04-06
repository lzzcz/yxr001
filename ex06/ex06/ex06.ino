// 定义LED引脚
const int ledPin_yellow = 4;  
const int ledPin_green = 17;

// 设置PWM属性
const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)

void setup() {
  Serial.begin(115200);

  // 【新版用法】直接将引脚、频率和分辨率绑定
  // 它会自动返回一个关联的通道（如果需要的话）
  ledcAttach(ledPin_yellow, freq, resolution);
  ledcAttach(ledPin_green, freq, resolution);
}

void loop() {
  // 逐渐变亮
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    // 【新版用法】直接通过引脚号写入，不再需要指定通道
    ledcWrite(ledPin_yellow, dutyCycle); 
    ledcWrite(ledPin_green, 255-dutyCycle);  
    delay(7);
  }

  // 逐渐变暗
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    ledcWrite(ledPin_yellow, dutyCycle); 
    ledcWrite(ledPin_green, 255-dutyCycle);   
    delay(7);
  }
  
  Serial.println("Breathing cycle completed");
}