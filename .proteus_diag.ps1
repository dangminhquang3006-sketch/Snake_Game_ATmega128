param([string]$Action="view",[int]$X=0,[int]$Y=0)
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public class ProteusDiagNative {
 [DllImport("user32.dll")] public static extern bool EnumWindows(EnumCallback cb,IntPtr l);
 [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h,StringBuilder b,int n);
 public static IntPtr FindMemory() {
  IntPtr result=IntPtr.Zero;
  EnumWindows(delegate(IntPtr h,IntPtr l) {StringBuilder b=new StringBuilder(256);GetWindowText(h,b,256);if(b.ToString().Contains("Data Memory - U1")) result=h;return true;},IntPtr.Zero);
  return result;
 }

 [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
 [DllImport("user32.dll")] public static extern IntPtr GetMenu(IntPtr h);
 [DllImport("user32.dll")] public static extern int GetMenuItemCount(IntPtr h);
 [DllImport("user32.dll")] public static extern IntPtr GetSubMenu(IntPtr h,int p);
 [DllImport("user32.dll")] public static extern uint GetMenuItemID(IntPtr h,int p);
 [DllImport("user32.dll")] public static extern int GetMenuString(IntPtr h,uint p,StringBuilder b,int n,uint f);
 public static void DumpMenu(IntPtr menu,string prefix) {
  for(int i=0;i<GetMenuItemCount(menu);i++) {
   StringBuilder b=new StringBuilder(256);GetMenuString(menu,(uint)i,b,256,0x400);
   string name=prefix+"/"+b.ToString();
   Console.WriteLine(GetMenuItemID(menu,i)+" "+name);
   IntPtr sub=GetSubMenu(menu,i);if(sub!=IntPtr.Zero) DumpMenu(sub,name);
  }
 }

 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h,IntPtr id);
 [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
 [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a,uint b,bool join);
 public static bool Activate(IntPtr h) {
  uint fg=GetWindowThreadProcessId(GetForegroundWindow(),IntPtr.Zero);
  uint me=GetCurrentThreadId();
  AttachThreadInput(me,fg,true);
  bool ok=SetForegroundWindow(h);
  AttachThreadInput(me,fg,false);
  return ok;
 }

 public delegate bool EnumCallback(IntPtr h,IntPtr l);
 [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr h,EnumCallback cb,IntPtr l);
 [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h,StringBuilder b,int n);
 [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr h,uint msg,IntPtr w,IntPtr l);
 public static IntPtr Target(IntPtr parent,int x,int y) {
  IntPtr result=parent; long area=long.MaxValue;
  EnumChildWindows(parent,delegate(IntPtr h,IntPtr l) {
   if(!IsWindowVisible(h)) return true;
   Rect r; GetWindowRect(h,out r);
   if(x>=r.Left && x<r.Right && y>=r.Top && y<r.Bottom) {
    long a=(long)(r.Right-r.Left)*(r.Bottom-r.Top);
    if(a>0 && a<area) {area=a;result=h;}
   }
   return true;
  },IntPtr.Zero);
  return result;
 }

 [StructLayout(LayoutKind.Sequential)] public struct Rect { public int Left,Top,Right,Bottom; }
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out Rect rect);
 [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
 [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
 [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
 [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h,int n);
 [DllImport("user32.dll")] public static extern bool SetCursorPos(int x,int y);
 [DllImport("user32.dll")] public static extern void mouse_event(uint f,uint x,uint y,uint d,UIntPtr e);
}
"@
[ProteusDiagNative]::SetProcessDPIAware() | Out-Null
$p=Get-Process -Name PDS | Select-Object -First 1

Start-Sleep -Milliseconds 500
if ($Action -eq "memaddr") {
 $mem=[ProteusDiagNative]::FindMemory()
 Write-Output "Memory window: $mem"
 [ProteusDiagNative]::Activate($mem) | Out-Null
 Start-Sleep -Milliseconds 200
 $target=[ProteusDiagNative]::Target($mem,500,500)
 $class=New-Object System.Text.StringBuilder 256
 [ProteusDiagNative]::GetClassName($target,$class,256) | Out-Null
 Write-Output "Memory target: $target $class"
 if ([ProteusDiagNative]::GetForegroundWindow() -ne $mem) { throw "Memory window is not focused" }
 $keys=New-Object -ComObject WScript.Shell
 [ProteusDiagNative]::SetCursorPos(821,350) | Out-Null
 for ($i=0;$i -lt 5;$i++) {
  [ProteusDiagNative]::mouse_event(2,0,0,0,[UIntPtr]::Zero)
  Start-Sleep -Milliseconds 50
  [ProteusDiagNative]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
  Start-Sleep -Milliseconds 100
 }
 Start-Sleep -Milliseconds 500
}
if ($Action -eq "memory") {
 [ProteusDiagNative]::Activate($p.MainWindowHandle) | Out-Null
 [ProteusDiagNative]::SendMessage($p.MainWindowHandle,0x111,[IntPtr]581,[IntPtr]::Zero) | Out-Null
 Start-Sleep -Milliseconds 400
 [ProteusDiagNative]::SendMessage($p.MainWindowHandle,0x111,[IntPtr]643,[IntPtr]::Zero) | Out-Null
 Start-Sleep -Milliseconds 400
 $target=[ProteusDiagNative]::Target($p.MainWindowHandle,500,500)
 $coords=[IntPtr](500 -bor (500 -shl 16))
 [ProteusDiagNative]::SendMessage($target,0x20A,[IntPtr](-2400 -shl 16),$coords) | Out-Null
 Start-Sleep -Milliseconds 400
}
if ($Action -eq "command") {
 [ProteusDiagNative]::Activate($p.MainWindowHandle) | Out-Null
 [ProteusDiagNative]::SendMessage($p.MainWindowHandle,0x111,[IntPtr]$X,[IntPtr]::Zero) | Out-Null
 Start-Sleep -Milliseconds 500
}
if ($Action -eq "menus") { [ProteusDiagNative]::DumpMenu([ProteusDiagNative]::GetMenu($p.MainWindowHandle),"") }
if ($Action -eq "zoom" -or $Action -eq "zoomout" -or $Action -eq "memscroll") {
 $target=[ProteusDiagNative]::Target($p.MainWindowHandle,$X,$Y)
 $coords=[IntPtr]($X -bor ($Y -shl 16))
 $wheel=240
 if ($Action -eq "zoomout") { $wheel=-240 }
 if ($Action -eq "memscroll") { $wheel=-2400 }
 [ProteusDiagNative]::SendMessage($target,0x20A,[IntPtr]($wheel -shl 16),$coords) | Out-Null
 Start-Sleep -Milliseconds 700
}
if ($Action -eq "realclick" -or $Action -eq "hold") {
 [ProteusDiagNative]::Activate($p.MainWindowHandle) | Out-Null
 if ([ProteusDiagNative]::GetForegroundWindow() -ne $p.MainWindowHandle) {
  throw "Proteus is not foreground; skipped click"
 }
 [ProteusDiagNative]::SetCursorPos($X,$Y) | Out-Null
 [ProteusDiagNative]::mouse_event(2,0,0,0,[UIntPtr]::Zero)
 Start-Sleep -Milliseconds 1000
 if ($Action -ne "hold") {
  [ProteusDiagNative]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
  Start-Sleep -Milliseconds 1000
 }
}
if ($Action -eq "release") {
 [ProteusDiagNative]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
 Start-Sleep -Milliseconds 500
}
if ($Action -eq "click") {
 $target=[ProteusDiagNative]::Target($p.MainWindowHandle,$X,$Y)
 $targetRect=New-Object ProteusDiagNative+Rect
 [ProteusDiagNative]::GetWindowRect($target,[ref]$targetRect) | Out-Null
 $class=New-Object System.Text.StringBuilder 256
 [ProteusDiagNative]::GetClassName($target,$class,256) | Out-Null
 Write-Output "Target: $target $class"
 $coords=[IntPtr](($X-$targetRect.Left) -bor (($Y-$targetRect.Top) -shl 16))
 [ProteusDiagNative]::SendMessage($target,0x200,[IntPtr]::Zero,$coords) | Out-Null
 [ProteusDiagNative]::SendMessage($target,0x201,[IntPtr]1,$coords) | Out-Null
 Start-Sleep -Milliseconds 750
 [ProteusDiagNative]::SendMessage($target,0x202,[IntPtr]::Zero,$coords) | Out-Null
 Start-Sleep -Milliseconds 750
}
$rect=New-Object ProteusDiagNative+Rect
[ProteusDiagNative]::GetWindowRect($p.MainWindowHandle,[ref]$rect) | Out-Null
Write-Output "Proteus window: $($rect.Left),$($rect.Top) - $($rect.Right),$($rect.Bottom)"
$shot=New-Object System.Drawing.Bitmap ($rect.Right-$rect.Left),($rect.Bottom-$rect.Top)
$graphics=[System.Drawing.Graphics]::FromImage($shot)
if ($Action -eq "desktop" -or $Action -eq "realclick" -or $Action -eq "hold" -or $Action -eq "command" -or $Action -eq "memscroll" -or $Action -eq "memory" -or $Action -eq "memaddr") {
 $graphics.CopyFromScreen(0,0,0,0,$shot.Size)
} else {
 $dc=$graphics.GetHdc()
 [ProteusDiagNative]::PrintWindow($p.MainWindowHandle,$dc,2) | Out-Null
 $graphics.ReleaseHdc($dc)
}
$shot.Save("D:\nhung\Snake_Game_ATmega128\.proteus_diagnostic.png")
$graphics.Dispose()
$shot.Dispose()
