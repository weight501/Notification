# Windows Toast Notification Library

![스크린샷 2024-12-29 184444](https://github.com/user-attachments/assets/624eded8-2ec9-4b16-899f-5f001d2f5e51)


Windows 10 이상에서 사용할 수 있는 토스트 알림을 쉽게 구현할 수 있는 DLL 라이브러리입니다. Unity, Unreal Engine 등 다양한 프레임워크에서 Windows 네이티브 토스트 알림을 쉽게 통합할 수 있습니다.

## 목차
- [기능](#기능)
- [요구사항](#요구사항)
- [설치 방법](#설치-방법)
- [사용 방법](#사용-방법)


## 기능
- **다양한 알림 템플릿 지원**
 - 단순 텍스트 알림
 - 제목과 메시지가 있는 알림
 - 두 줄 메시지가 있는 알림
 - 이미지가 포함된 알림
 - 
- **간단한 C 스타일 API**
 - C++, C#등 다양한 언어에서 쉽게 사용 가능
 - 단순한 함수 호출로 알림 표시

## 요구사항
### 런타임 요구사항
- Windows 10 이상
- [Visual C++ Redistributable for Visual Studio 2022](https://aka.ms/vs/17/release/vc_redist.x64.exe)

### 개발 요구사항
- Visual Studio 2022
- C++20 이상
- Windows SDK 10.0.19041.0 이상

## 설치 방법
### Visual Studio C++ 프로젝트
1. Notification.dll과 관련 파일을 다운로드
2. 프로젝트에 다음 파일 추가:
3. Visual Studio 프로젝트 설정:
   - 추가 포함 디렉터리에 `include` 경로 추가
   - 추가 라이브러리 디렉터리에 `lib` 경로 추가
   - 추가 종속성에 `Notification.lib` 추가

## 사용 방법
### Unity에서 사용하기
1. Unity 프로젝트의 `Assets/Plugins` 폴더에 `Notification.dll` 복사
2. C# 래퍼 클래스 생성:
```csharp
using System.Runtime.InteropServices;

public static class ToastNotification
{
    [DllImport("Notification.dll", CharSet = CharSet.Unicode)]
    public static extern bool Initialize(string appName, string aumi);

    [DllImport("Notification.dll", CharSet = CharSet.Unicode)]
    public static extern bool Show(string message, string title = null, 
                                 string message2 = null, string imagePath = null);
}
