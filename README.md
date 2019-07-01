# Air-quality-monitoring-system
Air quality monitoring system based on LoRa and Node.JS

LoRa통신과 Node.JS를 활용한 공기질 관리 시스템입니다.

![LoRa_modules](Pictures/V2_sensor.jpg = 500x)

LoRa 센서 노드


![LoRa_modules](Pictures/Server1.png)

센서값을 실시간으로 확인가능한 Node.JS 기반의 웹서비스


PCB를 비롯한 하드웨어는 TTGO LoRa V2보드를 기반으로 만들어졌습니다. 센서 노드는 BME280센서, GP2Y1023AU0F센서로부터 값을 읽어오고 게이트웨이에 값을 전송하며, 게이트웨이는 서버로 센서값을 올려서 DB에 추가하는 구조입니다.

### 수정사항
1. PCB의 DIP스위치와 연결된 0번 핀은 CLX1로 2.7V가 유지되며, 3.3V로 끌어올릴 시 업로딩이 안되며, 2번 핀의 경우 LED핀으로 0번, 2번 모두 다음 버전에서 수정 필요
1. PCB에서 GP2Y10 먼지센서가 PCB위에 올라갈 공간이 없으며 하단의 18650 배터리가 먼지센서의 구멍을 막음. 보드의 길이를 늘려야함
1. PCB에서 테스트용인 3.3V 및 GND핀들의 위치 수정 필요
1. 케이스에서 18650 홀더가 케이스와 충돌하여 길이를 늘려야함