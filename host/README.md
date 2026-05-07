# SSCB Host

WinForms CAN host application for the smart solid-state circuit breaker controller.

## Projects

- `SscbHost.Core`: CAN abstraction, ZLG `ControlCAN.dll` wrapper, SSCB protocol encode/decode.
- `SscbHost.WinForms`: engineering debug UI for connection, realtime data, parameter access, control commands, and fault logs.
- `SscbHost.Tests`: protocol parsing and frame-building tests.

## Build

Open [SscbHost.sln](/C:/Users/了女哦/Desktop/固态产品/需求文档/host/SscbHost.sln) in Visual Studio 2022 or later, or run:

```powershell
$env:DOTNET_CLI_HOME="$PWD\.dotnet-home"
$env:NUGET_PACKAGES="$PWD\.nuget-packages"
dotnet build host\SscbHost.WinForms\SscbHost.WinForms.csproj
```

## Runtime prerequisites

- Windows with `.NET Desktop Runtime 8`.
- ZLG driver installed.
- `ControlCAN.dll` available beside the WinForms executable, or in a directory on `PATH`.

## Default communication settings

- Device type: `4`
- Device index: `0`
- Channel index: `0`
- CAN bitrate: `500 kbps`
- Node ID: `1`
