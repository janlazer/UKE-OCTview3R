; Build through build-installer.ps1. Inno Setup 7.1 or later is required.
#ifndef PayloadDir
  #error PayloadDir must point to the prepared standalone payload.
#endif
#ifndef AppVersion
  #error AppVersion must match the application source.
#endif
#ifndef BuildOutputDir
  #error BuildOutputDir is required.
#endif
#ifndef AppIdValue
  #define AppIdValue "OCTview3R.Desktop.x64"
#endif

[Setup]
AppId={#AppIdValue}
AppName=OCTview3R
AppVersion={#AppVersion}
AppPublisher=Jan Hahn, Giovanno Moebes, Tammo Ripken
AppPublisherURL=https://github.com/janlazer/UKE-OCTview3R
AppSupportURL=https://github.com/janlazer/UKE-OCTview3R/issues
AppUpdatesURL=https://github.com/janlazer/UKE-OCTview3R/releases
DefaultDirName={localappdata}\Programs\OCTview3R
DefaultGroupName=OCTview3R
PrivilegesRequired=lowest
ArchitecturesAllowed=x64os
ArchitecturesInstallIn64BitMode=x64os
SetupArchitecture=x64
MinVersion=10.0
WizardStyle=modern
DisableProgramGroupPage=yes
AllowNoIcons=yes
OutputDir={#BuildOutputDir}
OutputBaseFilename=OCTview3R-{#AppVersion}-windows-x64-setup
SetupIconFile=..\OCTview3R\Resources\branding\octview3r.ico
UninstallDisplayIcon={app}\OCTview3R.exe
LicenseFile=..\LICENSE
InfoBeforeFile=INSTALLER_README.txt
Compression=lzma2
SolidCompression=yes
VersionInfoVersion={#AppVersion}.0
VersionInfoDescription=OCTview3R Windows installer
VersionInfoProductName=OCTview3R
VersionInfoProductVersion={#AppVersion}
CloseApplications=yes
RestartApplications=no
SetupMutex=OCTview3R.Setup.x64
ChangesEnvironment=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#PayloadDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

#ifndef InstallerTestBuild
[Icons]
Name: "{autoprograms}\OCTview3R"; Filename: "{app}\OCTview3R.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\OCTview3R"; Filename: "{app}\OCTview3R.exe"; WorkingDir: "{app}"; Tasks: desktopicon
#endif

[Run]
Filename: "{app}\OCTview3R.exe"; Description: "{cm:LaunchProgram,OCTview3R}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent unchecked

; No wildcard cleanup: uninstall removes only installed files, preserving
; additional user files and the application's existing per-user settings.
