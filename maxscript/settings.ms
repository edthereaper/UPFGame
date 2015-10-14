rollout ROLLOUT_VINE_settings "VINE tools settings" width:688 height:104
(
	label lbl1 "Save folder" pos:[8,8] width:72 height:13
	edittext data_path "" pos:[120,8] width:552 height:16
	button saveBrowse "..." pos:[88,8] width:24 height:16 toolTip:"Browse..."
	label lbl2 "Tools folder" pos:[8,32] width:72 height:13
	edittext tools_path "" pos:[120,32] width:552 height:16
	button toolsBrowse "..." pos:[88,32] width:24 height:16 toolTip:"Browse..."
	label lbl3 "Meshes folder" pos:[16,72] width:72 height:13
	edittext meshes_path "" pos:[120,72] width:552 height:16
	button meshesBrowse "..." pos:[88,72] width:24 height:16 toolTip:"Browse..."
	groupBox lvlEditorGB "Level Editor" pos:[8,56] width:672 height:40
	
	fn validatePath dir = (
		if findString dir " " != undefined then (
			messagebox "Can't use paths with spaces!\nPlease use a different path." \
				title:"Artist strikes again..."
		)
		local lastChar = dir[dir.count]
		if  lastChar != "\\" and lastChar != "/"  then (
			append dir "\\"
		)
		return dir
	)
		
	fn saveDataPath = (
		if data_path.text != "" then (
			data_path.text = validatePath data_path.text 
			setINISetting VINE_settingsFile "settings" "data_path" data_path.text
			VINE_dataPath = meshes_path.text
		)
	)	
	fn saveToolsPath = (
		if tools_path.text != "" then (
			tools_path.text = validatePath tools_path.text 
			setINISetting VINE_settingsFile "settings" "tools_path" tools_path.text
			VINE_toolsPath = meshes_path.text
		)
	)
	fn saveMeshesPath = (
		if meshes_path.text != "" then (
			meshes_path.text = validatePath meshes_path.text 
			setINISetting VINE_settingsFile "Level Editor" "meshes_path" meshes_path.text
			VINE_meshesPath = meshes_path.text
		)
	)
		
	on ROLLOUT_VINE_settings open do
	(
			data_path.text = validatePath data_path.text 
			tools_path.text = validatePath tools_path.text 
			meshes_path.text = validatePath meshes_path.text 
		
			if (VINE_dataPath != undefined)
			then data_path.text = getINISetting VINE_settingsFile "settings" "data_path"
			else data_path.text = maxFilePath 
			
			if (VINE_toolsPath != undefined)
			then tools_path.text = getINISetting VINE_settingsFile "settings" "tools_path"
			else tools_path.text = maxFilePath
		
			if (VINE_meshesPath != undefined)
			then meshes_path.text = getINISetting VINE_settingsFile "Level Editor" "meshes_path"
			else meshes_path.text = maxFilePath
		)
	on ROLLOUT_VINE_settings close do
	(
		saveDataPath()
		saveToolsPath()
		saveMeshesPath()
		)
	on data_path entered text do
	(
		saveDataPath()
	)
	on saveBrowse pressed do
	(
			newPath = getSavePath \
				caption:"Select the export destination." \
				initialDir:data_path.text
			if newPath != undefined then data_path.text = newPath
			saveDataPath()
		)
	on tools_path entered text do
	(
		saveToolsPath()
	)
	on toolsBrowse pressed do
	(
			newPath = getSavePath \
				caption:"Select the folder where the tools (like texconv.exe) are." \
				initialDir:tools_path.text
			if newPath != undefined then tools_path.text = newPath
			saveToolsPath()
		)
	on meshes_path entered text do
	(
		saveMeshesPath()
	)
	on meshesBrowse pressed do
	(
			newPath = getSavePath \
				caption:"Select the folder where the level editor meshes reside." \
				initialDir:meshes_path.text
			if newPath != undefined then meshes_path.text = newPath
			saveMeshesPath()
		)
)

fn LOAD_MACRO_VINE_settings = (
	macroScript MACRO_VINE_settings
		category:"VINEDETTA"
		toolTip: "Settings"
		icon:#("settings", 1)
	(
		addrollout ROLLOUT_VINE_settings \
			(newRolloutFloater "VINE tools settings  --- (Close to save changes)" 700 120) \
			border:false
	)
	OK
)
LOAD_MACRO_VINE_settings()