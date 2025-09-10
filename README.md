#  Interactive scene using DirectX11

시연 영상(클릭)<br>
[![Video Label](http://img.youtube.com/vi/Cyxn7jwYV2s/0.jpg)](https://youtu.be/Cyxn7jwYV2s)
<iframe>https://youtu.be/Cyxn7jwYV2s</iframe>

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
- FBX모델 사용하고(assimp) 각 모델에 멀티텍스쳐링 적용
- FBX 파일 처리 및 텍스쳐, 인스턴싱에 관한 모든 모델 정보를 외부 json파일에서 관리(rapidjson)
- 모델 로드를 쓰레드 풀을 사용하여 멀티쓰레딩 적용
- 로딩 화면 구성하고 로딩 과정 시각화
- 차량 모델에 대해 마우스 피킹 적용
- 피킹으로 고정된 차량에 카메라를 부착, 같이 움직이며 조작할 수 있도록 설정
- 차량 모델의 움직임에 대하여 현실적인 가속, 조향 적용(하드코딩)

---

**어려웠던 점**
- Collision 구현 단계에서 각 모델별 기본 크기와 회전값이 달라  mesh기반 충돌 처리를 하지 않는 이상 실제 보이는 것과 다른 위치에서 충돌 처리가 일어났다. 하나하나 다 박스를 지정해주어야 하는데 적용에 해답을 찾지 못했다.
- DirectFont 사용을 위해  D2D와 기존 D3D render buffe가 호환되지 않아 flickering이 발생했다. 결국 d3dclass의 버퍼 관련 부분을 전부 수정했다.
- 아직도 원인을 찾지 못했는데, 쉐이더 코드에서 point light 설정을 할 때 개별적으로 하지 않으면 작동하지 않았다. point light를 위한 버퍼를 따로 만들고 각각의 point light를 순차적으로 설정하여 해결했다.
- 적당한 polygon을 가진 무료 모델을 찾고 수정하는 것에 절반의 시간이 들어갔다.
- Multi-texturing을 구현하는 과정에서 텍스쳐 없이 기본 색상만으로 만들어진 모델은 호환되지 않는 방식으로 코드를 만들어서 일부 모델들은 직접 텍스쳐를 만들어 넣어주어야 했다.
- 한 개의 텍스쳐 파일로 여러 부위를 적용하는 모델은 같은 텍스쳐 파일을 중복해 등록해야만 하는 불편함이 있다.
