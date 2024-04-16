@setlocal
cd /d %~dp0

mkdir temp2
cd temp2
scs_packer create archive_b.scs -root ..\data
%~dp0\..\..\bin\win_x86\converter_pix_d -b archive_b.scs -extract_d / -e %cd%\data_extracted
scs_packer create archive_b2.scs -root data_extracted

@endlocal