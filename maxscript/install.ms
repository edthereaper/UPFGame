fileIn "constants.ms"
fileIn "tools.ms"

struct Vinedetta_Installation (
	scriptsDir = (getDir #userStartupScripts ) + "\\vinedetta\\",
	iconsDir = (getDir #userIcons) + "\\",
	currentDir,
	startupFiles = #(
		"tools.ms",
		"constants.ms",
		"settings.ms",
		"levelVars.ms",
		"levelObjects.ms",
		"levelPieces.ms",
		"levelKits.ms",
		"levelLogic.ms",
		"exportLevel.ms",
		"exportProperties.ms",
		"batchExportMesh.ms",
		"export_mesh.ms",
		"export_camera.ms"
		"export_anim.ms"
	),
	iconFiles = #(
		"exportMesh_24i.bmp",
		"exportMesh_24a.bmp",
		"exportMesh_16i.bmp",
		"exportMesh_16a.bmp",
		"exportBatch_24i.bmp",
		"exportBatch_24a.bmp",
		"exportBatch_16i.bmp",
		"exportBatch_16a.bmp",
		"exportMat_24i.bmp",
		"exportMat_24a.bmp",
		"exportMat_16i.bmp",
		"exportMat_16a.bmp",
		"kits_24i.bmp",
		"kits_24a.bmp",
		"kits_16i.bmp",
		"kits_16a.bmp",
		"collision_24i.bmp",
		"collision_24a.bmp",
		"collision_16i.bmp",
		"collision_16a.bmp",
		"settings_24i.bmp",
		"settings_24a.bmp",
		"settings_16i.bmp",
		"settings_16a.bmp",
		"exportProps_24i.bmp",
		"exportProps_24a.bmp",
		"exportProps_16i.bmp",
		"exportProps_16i.bmp",
		"toggleTriggers_24i.bmp",
		"toggleTriggers_24a.bmp",
		"toggleTriggers_16i.bmp",
		"toggleTriggers_16i.bmp",
		"toggleSpatial_24i.bmp",
		"toggleSpatial_24a.bmp",
		"toggleSpatial_16i.bmp",
		"toggleSpatial_16i.bmp",
		"cam_icon_16i.bmp",
		"cam_icon_24i.bmp",
		"export_animation_16i.bmp",
		"export_animation_24i.bmp"
		

	),                 
	
	fn deletePreviousfiles = (
		--format "Deleting previous files:\n"
		for file in getfiles(scriptsDir + "*") do (
			--format "- %\n" file
			deleteFile file
		)
		for file in getfiles(iconsDir + "*") do (
			--format "- %\n" file
			deleteFile file
		)
	),
	
	fn uninstall = (
		deletePreviousfiles()
		if doesFileExist VINE_settingsFile then (
			--format "- %\n" VINE_settingsFile
			local _ = deleteFile VINE_settingsFile
		)
		OK
	),
	
	fn copyStartupFiles = (		
		-- Make sure the directories exist
		if not doesDirectoryExist scriptsDir then (
			makeDir scriptsDir
		)
		if not doesDirectoryExist iconsDir then (
			makeDir iconsDir
		)
		-- For each *.ms in the directory where this script is
		--format "Copying:\n"
		for s in startupFiles do (
			local file = currentDir + s
			
			--format "- %%... " scriptsDir s
			local fileDest = (scriptsDir + s)
			
			-- Copy the file
			copyFile file fileDest
			--if copyFile file fileDest
			--then format "OK\n"
			--else format "FAILED\n" 
		)
		for f in iconFiles do (
			local file = currentDir + f
			
			--format "- %%... " iconsDir f
			local fileDest = (iconsDir + f)
			
			-- Copy the file
			copyFile file fileDest
			--if copyFile file fileDest
			--then format "OK\n"
			--else format "FAILED\n" 
		)
		OK
	),
	
	fn runStartupfiles = (
		--format "Running startup files (so values are loaded now):\n"
		for file in startupFiles do (
			--format "- %\n" file
			fileIn file
		)
	),

	fn makePropsFile = (
		local property
		property = getINISetting VINE_settingsFile "settings" "data_path"
		if property == "" then (
			setINISetting VINE_settingsFile "settings" "data_path" maxFilePath
		)
		property = getINISetting VINE_settingsFile "settings" "tools_path"
		if property == "" then (
			setINISetting VINE_settingsFile "settings" "tools_path" (getFilenamePath(getThisScriptFilename()) + "tools\\")
		)
		property = getINISetting VINE_settingsFile "Level Editor" "meshes_path"
		if property == "" then (
			setINISetting VINE_settingsFile "Level Editor" "meshes_path" (getFilenamePath(getThisScriptFilename()) + "levelMeshes\\")
		)
	),
	
	fn install = (
		currentDir = getFilenamePath(getThisScriptFilename())
		local thisScript = getThisScriptFilename()
		if thisScript != undefined then (
			local readme = getFilenamePath(thisScript)+"README.txt"
			if doesFileExist readme then (
				actionMan.executeAction 0 "40472" --Open Listener
				cat readme
				format "\n"
			)
		)
		
		makePropsFile()
		deletePreviousFiles()
		copyStartupFiles()
		runStartupFiles()
		colorman.reInitIcons()
		OK
	)
)

fn VINE_uninstall = (
	local a = Vinedetta_Installation()
	a.uninstall()
)

fn run = (
	clearListener()
	local a = Vinedetta_Installation()
	a.install()
	messagebox "Install complete." title:VINE_version beep:false
	OK
)
run()



