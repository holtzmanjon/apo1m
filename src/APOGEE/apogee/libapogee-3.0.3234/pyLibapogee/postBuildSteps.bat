echo %1
rmdir /S /Q %1
mkdir %1
copy %2\_pylibapogee.pyd %1
copy %2\libapogee.dll %1
copy %2\pylibapogee.py %1
copy %3\__init__.py %1
copy %3\pylibapogee_setup.py %1

cd %4

echo building  mainTest executable
call cxfreeze mainTestDriver.py --target-dir dist  --include-modules sip

echo after build mainTest executable