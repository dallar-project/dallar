#	Build MSI packages

MSB=%ProgramFiles(x86)%\MSBuild\14.0\Bin\amd64\msbuild.exe
WIX_LINK = light.exe -ext WixUIExtension -ext WixUtilExtension -dWixUILicenseRtf=doc\dallar_lic.rtf -dWixUIDialogBmp=share\pixmaps\wix-banner.bmp -dWixUIBannerBmp=share\pixmaps\wix-topbanner.bmp


all : dallar_x86.msi dallar_x64.msi

src\qt\res_bitcoin.cpp : src\qt\moc.proj
	cd src\qt
	msbuild moc.proj
	cd ..\..


x86_R_St\dallar-qt.exe : src\dallar.cpp src\qt\res_bitcoin.cpp
	""$(MSB)"" dallar.sln /p:Configuration=R_St,Platform=x86 /v:n

x64_R_St\dallar-qt.exe : src\dallar.cpp src\qt\res_bitcoin.cpp
	""$(MSB)"" dallar.sln /p:Configuration=R_St,Platform=x64 /v:n


dallar_x86.msi : dallar.wxs x86_R_St\dallar-qt.exe
	candle.exe -o dallar-x86.wixobj dallar.wxs
	$(WIX_LINK)  -out $@ dallar-x86.wixobj

dallar_x64.msi : dallar.wxs x64_R_St\dallar-qt.exe
	candle.exe -arch x64 -o dallar-x64.wixobj dallar.wxs
	$(WIX_LINK) -out $@ dallar-x64.wixobj
