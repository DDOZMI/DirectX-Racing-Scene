#  Interactive scene using DirectX11

시연 영상(클릭)<br>
[![Video Label](http://img.youtube.com/vi/Cyxn7jwYV2s/0.jpg)](https://youtu.be/Cyxn7jwYV2s)

**내용**
- 다수의 모델과 인스턴싱을 이용해 씬 구성
- Phong light와 3개의 Point light 설정/거리에 따른 감쇠 효과 적용
- D2D/DirectWrite를 이용하여 폰트 렌더
- CPU로드, FPS를 포함한 화면 정보와 씬 설명 배치
- 일부 모델에 대해 자전과 공전 적용
- 이동이 가능한 카메라 배치
- 키보드 입력으로 화면 설정(Render, Filter, Culling, Lighting) 변경 가능
- 빌보딩으로 원경의 나무 이미지 배치
- 배열 구조로 하드코딩한 객체를 Vector 리팩터링
- 각 모델에 대해서 멀티텍스쳐링 적용
- FBX 파일 처리 및 텍스쳐, 인스턴싱에 관한 모든 모델 정보를 외부 json파일에서 관리
- 모델 로드를 쓰레드 풀을 사용하여 멀티쓰레딩 적용
- 로딩 화면 구성하고 로딩 과정 시각화
- 차량 모델에 대해 마우스 피킹 적용
- 피킹으로 고정된 차량에 카메라를 부착, 같이 움직이며 조작할 수 있도록 설정
- 차량 모델의 움직임에 대하여 현실적인 가속, 조향 적용(하드코딩)
