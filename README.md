# Air-quality-monitoring-system
Air quality monitoring system based on LoRa and Node.JS

### PCB 문제
1. DIP스위치와 연결된 0번 핀은 CLX1로 2.7V가 유지되며, 3.3V로 끌어올릴 시 업로딩이 안되며, 2번 핀의 경우 LED핀으로 0번, 2번 모두 다음 버전에서 수정 필요
1. GP2Y10 먼지센서가 PCB위에 올라갈 공간이 없으며 하단의 18650 배터리가 먼지센서의 구멍을 막음. 보드의 길이를 늘려야함
1. 테스트용인 3.3V 및 GND핀들의 위치 수정 필요
1. 3mm 마운팅 홀에 plated-through hole 및 GND플레인 강화 via 추가필요

