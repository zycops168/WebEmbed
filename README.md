# WebEmbed
## Web Interface สำหรับ config SSID/PASS WiFi
## การออกแบบการใช้งานเมื่อมีการย้ายอุปกรณ์ไปติดตั้งในสถานที่ใหม่
### 1 เมื่ออุปกรณ์ทำงาน จะเปิดโหมด STA 10 วินาทีโดยเชื่อมต่อไปที่ AP ssid, pass ล่าสุดที่กำหนดไว้(ดึงมาจาก EEPROM) 
  ระหว่างที่พยายามเชื่อมต่อ STA led ที่อุปกรณ์ จะกระพิบติดดับ สลับไปมา ถ้าเชื่อมต่อไม่ได้ให้เข้าโหมด AP เมื่อเชื่อมต่อสำเร็จไฟ led จะไม่กระพิบ
### 2 การเข้าโหมด AP เชื่อมต่อ AP ผ่าน WiFi ชื่อ ESP32-AP-[chip_id] ( chip_id  อาจจะแปะไว้ที่กล่องของอุปกรณ์) 
  chip_id ดึงมาจาก getEfuseMac() เพื่อต้องการให้ ชื่อ ssid สามารถระบุว่าเป็นของอุปกรณ์ตัวไหนได้ และรองรับ multiple device 
### 3 เมื่อเชื่อมต่อ AP ได้แล้ว เข้าไปที่ ip 192.168.4.1 เพื่อทำการ config ssid, pass ที่ จะให้อุปกรณ์ไปเชื่อมต่อ 
  (เมื่อมีการ call API led บนตัวบอร์ดก็จะกระพิบ) โดยฝั่ง  server จะมี web service สำหรับการ get post ข้อมูล ssid, pass มาแสดงในหน้า เว็บและส่งไปเก็บที่ตัวอุปกรณ์
### 4 ออกแบบปุ่ม clear ไว้เพื่อทดสอบการ reload ข้อมูลหลังจากกดปุ่ม clear 
### 5 หลังจากกำหนด ssid pass ใหม่เรียบร้อยแล้ว ข้อมูลที่ถูก submit เข้ามาจะถูกเขียนไว้ ใน EEPROM ให้ทำการ reset อุปกรณ์เพื่อเชื่อมต่อกับ WiFi ที่เรากำหนด ssid, pass ใหม่