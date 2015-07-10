-- Set true to test luarocks installation, set to false if testing local development changes.
_G['_MONGOROVER_RELEASE'] = false 

if _G["_MONGOROVER_RELEASE"] == false then
	print("****************************************************************************************************************")
	print("In test/setReleaseType.lua, the _G['_MONGOROVER_RELEASE'] variable is set to false")
	print("testing local debug build")
	print("if this is erroring, you need to use cmake to create a local compiled build")
	print("if just testing luarocks build, please set this variable to true")
	print("****************************************************************************************************************")
end
