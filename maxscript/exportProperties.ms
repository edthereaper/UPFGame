fn LOAD_MACRO_VINE_exportProps= (
	macroScript MACRO_VINE_exportProps
		category:"VINEDETTA"
		toolTip: "Export user properties to file."
		icon:#("exportProps", 1)
	(
		if $ == undefined then throw "No object selected."
		if maxfilepath == "" or maxfilename == "" then throw "Unsaved file!"
		local props = getUserPropBuffer $
		if props == OK or props == "" then throw "No user properties to export!"
		gc()
		local filename = maxfilepath + (getFilenamefile maxfilename) + ".properties"
		local fs = openFile filename mode:"w"
		format "%\n" props to:fs
		close fs
		gc()
	)
	OK
)
LOAD_MACRO_VINE_exportProps()

fn VINE_writeUserProps obj fs:listener = (
	local props = getUserPropBuffer obj
	if not props == OK or props == "" then
		for str in (filterString props "\n\r ") do format " %" str to:fs
)



