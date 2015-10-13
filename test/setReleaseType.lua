-- Set true to test luarocks installation, set to false if testing local development changes.
_G['__MONGOROVER_TEST_ENVIRONMENT'] = nil

if _G["__MONGOROVER_TEST_ENVIRONMENT"] then
	print("****************************************************************************************************************")
	print("In test/setReleaseType.lua, the _G['__MONGOROVER_TEST_ENVIRONMENT'] variable is set to true")
	print("testing local debug build")
	print("if this is erroring, you need to use cmake to create a local compiled build")
	print("if just testing luarocks build, please set this variable to true")
	print("****************************************************************************************************************")
end
