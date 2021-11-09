project "XYZScript"
		kind "SharedLib"
		language "C#"
			
		targetdir ("%{wks.location}/XYZEditor/Assets/Scripts")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"%{wks.location}/XYZEditor/Assets/**.cs"
		}

		links
		{
			"XYZScriptCore"
		}