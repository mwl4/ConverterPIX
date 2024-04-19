@setlocal
cd /d %~dp0

mkdir temp
cd temp
scs_packer create archive_a.scs -root ..\data
scs_packer extract archive_a.scs -root data_extracted
scs_packer create archive_a2.scs -root data_extracted

@endlocal