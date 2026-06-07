;InnoSetupVersion=5.3.10 (Unicode)

[Setup]
AppName=Advanced Serial Port Terminal
AppVerName=Advanced Serial Port Terminal 6 (Build 6.0.382)
AppId=Advanced Serial Port Terminal
AppPublisher=ELTIMA Software
DefaultDirName={pf}\Eltima Software\Advanced Serial Port Terminal
DefaultGroupName=Eltima Software\Advanced Serial Port Terminal
UninstallDisplayIcon={app}\Terminal.exe
OutputBaseFilename=serial_port_terminal
Compression=lzma2
AllowNoIcons=yes
LicenseFile=embedded\License.rtf
WizardImageFile=embedded\WizardImage0.bmp
WizardSmallImageFile=embedded\WizardSmallImage0.bmp

[Files]
Source: "{app}\Terminal.exe"; DestDir: "{app}"; MinVersion: 0.0,5.0; Flags: ignoreversion 
Source: "{app}\GdiPlus.dll"; DestDir: "{app}"; MinVersion: 0.0,5.0; OnlyBelowVersion: 0.0,5.01; Flags: ignoreversion 

[Registry]
Root: HKCU; Subkey: "Software\Eltima\Advanced Serial Port Terminal 6"; MinVersion: 0.0,5.0; Flags: uninsdeletekey 

[Run]
Filename: "{app}\Terminal.exe"; Description: "Run Advanced Serial Port Terminal 6"; MinVersion: 0.0,5.0; Flags: postinstall nowait
Filename: "http://wiki.eltima.com/user-guides/serial-port-terminal/start.html"; Description: "Open online quick starting guide"; MinVersion: 0.0,5.0; Flags: shellexec postinstall unchecked nowait

[UninstallRun]
Filename: "http://www.eltima.com/uninstall/ymuyivekihuntcqn/6.0.382/"; MinVersion: 0.0,5.0; Flags: shellexec nowait

[Icons]
Name: "{group}\Advanced Serial Port Terminal"; Filename: "{app}\Terminal.exe"; WorkingDir: "{app}"; Comment: "Advanced Serial Port Terminal is a session-based, multi-purpose application that provides simple communication interface to connect to any serial port device"; MinVersion: 0.0,5.0; 
Name: "{group}\Uninstall Advanced Serial Port Terminal"; Filename: "{uninstallexe}"; Comment: "Completely removes the installed application from your system."; MinVersion: 0.0,5.0; 
Name: "{group}\Helpful resources\Read online user manual"; Filename: "http://wiki.eltima.com/user-guides/serial-port-terminal.html"; Comment: "Opens online HTML version of the User Manual, requires Internet connection"; MinVersion: 0.0,5.0; 
Name: "{group}\Helpful resources\Advanced Serial Port Terminal homepage"; Filename: "http://www.eltima.com/products/serial-port-terminal/"; Comment: "Opens Advanced Serial Port Terminal homepage, requires Internet connection"; MinVersion: 0.0,5.0; 
Name: "{group}\Helpful resources\Other Eltima products"; Filename: "http://www.eltima.com/products/"; MinVersion: 0.0,5.0; 
Name: "{group}\Helpful resources\Visit our Knowledge Base at Eltima Wiki"; Filename: "http://wiki.eltima.com/knowledge-base.html"; MinVersion: 0.0,5.0; 
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Launch Advanced Serial Port Terminal"; Filename: "{app}\Terminal.exe"; WorkingDir: "{app}"; IconFilename: "{app}\Terminal.exe"; Comment: "Advanced Serial Port Terminal is a session-based, multi-purpose application that provides simple communication interface to connect to any serial port device"; Tasks: quicklaunchicon; MinVersion: 0.0,5.0; 
Name: "{userdesktop}\Advanced Serial Port Terminal"; Filename: "{app}\Terminal.exe"; WorkingDir: "{app}"; IconFilename: "{app}\Terminal.exe"; Tasks: desktopicon; MinVersion: 0.0,5.0; 

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 0.0,5.0; 
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; MinVersion: 0.0,5.0; 

[CustomMessages]
default.NameAndVersion=%1 version %2
default.AdditionalIcons=Additional icons:
default.CreateDesktopIcon=Create a &desktop icon
default.CreateQuickLaunchIcon=Create a &Quick Launch icon
default.ProgramOnTheWeb=%1 on the Web
default.UninstallProgram=Uninstall %1
default.LaunchProgram=Launch %1
default.AssocFileExtension=&Associate %1 with the %2 file extension
default.AssocingFileExtension=Associating %1 with the %2 file extension...

[Languages]
; These files are stubs
; To achieve better results after recompilation, use the real language files
Name: "default"; MessagesFile: "embedded\default.isl"; 
