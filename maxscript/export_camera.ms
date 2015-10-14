filein "tools.ms"

struct VINE_cameraExporter (

	fn getDataPath = (
			local ret = getINISetting VINE_settingsFile "settings" "data_path"
			if ret == "" then throw("No data path set.")
			return ret
		),

	fn exportCamera camera1 = (

		--valid camera
		if camera1 == undefined then throw "I need the camera object!"
		
		--path of the camera file
		local level = getUserProp camera1 "level"
		local data_path = getDataPath()
		local out_path = data_path + "camera\\" + "level" + level as string +"\\"
		
		ensurePathExists(getFilenamePath(out_path))
		
		local filename = out_path + camera1.name + ".anim"
		local count = 1
		
		local fs = fopen filename "w+"
		if fs == undefined then throw ( "Can't create camera animfile output file " + filename )

		-- identify
		local camera_magic = 0xfedcba00
		
		-- storage keyframes
		local nkeys = animationRange.end - animationRange.start + 1
		local animation_duration = ( nkeys as float ) / 4800		-- Convert frames to ticks and ticks to second
		-- Save header
		WriteLong fs camera_magic
		WriteLong fs nkeys 						-- per object
		WriteFloat fs animation_duration		-- In seconds
		WriteLong fs 0 

		-- Write for each key, where is each objs
		local w = animationRange.start
		while w <= animationRange.end do (
			
			--format "Exporting key %\n"w
			-- For each obj
		
				local posC = at time w camera1.position	
				local posCmcv = max2mcv posC
				WriteFloat fs posCmcv.x
				WriteFloat fs posCmcv.y
				WriteFloat fs posCmcv.z
				WriteFloat fs 0 			-- Dummy

				----------------------------------------------------------------------
				
				
				local posT = at time w camera1.target.position	
				local posTmcv = max2mcv posT
				WriteFloat fs posTmcv.x
				WriteFloat fs posTmcv.y
				WriteFloat fs posTmcv.z
				WriteFloat fs 0 			-- Dummy
				
				--format "At Time % pos:% taget:%\n" w  posCmcv posTmcv
			
			w = w + 1
		)
		
		format "Exporting % camera and target with % keys\n" camera1.name nkeys
		-- Cerrar fichero
		fclose fs
		OK
	),

	fn getCamerasInScene = (
		for c in cameras collect c = (
			if findstring c.name "target" == undefined then
				exportCamera c 
			)
	),

	fn exportCameras = (
		
		--path of the camera file
		local data_path = getDataPath()
		local out_path = data_path + "camera\\"
		
		ensurePathExists(getFilenamePath(out_path))
		
		local file_name = out_path + getFilenameFile maxfilename + ".xml"
		
		-- Open file in binary mode, destroy existing version
		
		
		local fs_exporter = openFile file_name mode:"w+"
		if fs_exporter == undefined then throw ( "ERROR: can't open "+file_name+" for rewrite.")
		
		format "<camera>\n" to:fs_exporter
		local idx = 0
		for c in cameras collect c = (
			if findstring c.name "target" == undefined do (
				local maxFrame = getUserProp c "max"
				local level = getUserProp c "level"
				format "\t<cinematic name=\"%\" level=\"%\" max=\"%\" idx=\"%\" />\n" c.name level maxFrame idx to:fs_exporter
				idx+=1
			)
			)
		format "</camera>" to:fs_exporter
		close fs_exporter
		messageBox "Camera exported successfull"
	),


	fn testExportCameras = (
		
		local cameras_ = cameras
		
		if cameras_.count  >  0 then (
		gc()
		getCamerasInScene()
		exportCameras()
		)
		else
		(
			messageBox "Need cameras in scene"
		)
		
	)
)
	
fn LOAD_MACRO_VINE_staticCameraExporter = (
	macroScript MACRO_VINE_staticCameraExporter
		category:"VINEDETTA"
		toolTip: "Camera Exporter"
		icon:#("cam_icon", 1)
	(
		local exporter = VINE_cameraExporter()
		exporter.testExportCameras()
		OK
	)
)
	
LOAD_MACRO_VINE_staticCameraExporter()