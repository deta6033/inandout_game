# InAndOut C++ 전투/UI 개선 브랜치 적용 안내

브랜치: `feat/combat-ui-ai-overhaul` (기존 main 보존)

## 변경된 C++ 코드
- SonCharacter: 베개 / 폼다트 총 장착 전환, 좌클릭 무기별 공격, 궁극기 충전 소비 후 기절, 전환 시 메시 표시 및 색상 파라미터
- MomCharacter/MomAIController: 근접·달걀·궁극기 공격 예고/회복 타이머, 기절 시 예고 취소, 보스 색상 파라미터
- InAndOutHUD: 현재 장착 무기 슬롯 및 무기 전환 힌트 표시
- Config/DefaultInput.ini: 좌클릭 공격, 1/2 또는 휠로 무기 전환

## 꼭 해야 할 순서
1. GitHub Desktop에서 Fetch origin → 브랜치 `feat/combat-ui-ai-overhaul` 선택 → Pull origin. 기존 에디터는 닫은 상태 권장.
2. .uproject 파일을 열고 C++ 재빌드. 실패 시 가장 위의 UnrealBuildTool 컴파일 오류부터 확인.
3. 플레이 테스트: 1(베개), 2(폼다트), 휠, 좌클릭, R 재장전, Q 성적표 궁극기, 엄마 경고 → 공격 → 회복.
4. `BP_Son`: FoamDartGunMesh / PillowMesh가 실제 머티리얼을 사용하는지 확인. 머티리얼 인스턴스의 벡터 파라미터 `TintColor`를 생성/연결해야 C++ 색상 값이 화면에 나타남. 무기 메시가 비어 있으면 먼저 메시를 지정.
5. `BP_Mom`: 캐릭터 메시 머티리얼에 `TintColor` 벡터 파라미터 추가. 메시가 여러 슬롯이라면 각 슬롯 머티리얼 점검.
6. `BP_Mom` 이벤트: On Boss Attack Warning(AttackIndex 0=고무장갑,1=달걀,2=잔소리) 이벤트로 공격 예고 FX/사운드/애니메이션 추가, On Boss Attack Recovery 이벤트로 회복 연출 추가.
7. UMG: 현재 HUD에 무기 슬롯은 C++ Canvas로 추가되었으나, 전체 HUD/메뉴의 고품질 최종 레이아웃은 아직 UMG 위젯 에셋 디자인이 필요. 반응형 앵커/DPI 및 메뉴 위젯 제작 후 기존 Canvas HUD와 중복 노출을 피할 것.
8. 16:9 및 16:10 화면에서 UI 잘림 확인. 보스 기절/사망/공격 예고 동시 발생, 플레이어 사망/재시작, 재장전 도중 무기 전환 테스트.

## 기능상 주의
- 폼다트 재장전은 베개로 전환해도 백그라운드에서 계속 진행되는 정책으로 구현됨.
- 성적표는 무기 슬롯과 분리된 궁극기(Q)로 유지함.
- 본 브랜치는 저장소 코드 정적 수정만 한 상태. 실제 Unreal Editor/Windows C++ 빌드 및 게임 플레이 테스트는 아직 수행하지 않았음.
- `main`으로 합치기 전에 컴파일과 게임 플레이 검증을 반드시 완료할 것.
