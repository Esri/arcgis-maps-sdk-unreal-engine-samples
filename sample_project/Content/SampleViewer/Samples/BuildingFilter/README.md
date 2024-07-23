# Building Filter

Allows users to toggle the visibility of different levels, construction phases, disciplines, and categories within a building scene layer. This sample demonstrates how to explore building scene layers and filter based on various criteria.

![image](https://github.com/user-attachments/assets/f2fa6633-8227-4d89-8745-38111461662d)

## How to use the sample (SampleViewer)
1. The SampleViewer scene should open by default. If it is not open, click the **SampleViewer** scene to open it.
2. Click play.
3. Using the UI, enter an APIKey in the input field to the top left.
4. Open the **Samples** drop-down and click **Building Filter** to open the level.
5. Use the UI to toggle visibility of different levels, construction phases, disciplines, and categories.

## How to use the sample (BuildingFilter Scene)
1. Open the **BuildingFilter** Level.
2. Click on the **ArcGISMapActor** in the outliner and set your API key in the **Details** panel.
3. Click play.
4. Use the UI to toggle visibility of different levels, construction phases, disciplines, and categories.

## How it works
1. Create an ArcGISMap actor with a building scene layer.
2. Create a default building filter to apply different where clauses.
3. Create an actor with a C++ script to filter different criteria.
4. The [`GetSubLayers`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/buildingscene/arcgisbuildingscenesublayer/#sublayers) function is used to fetch a collection of the building scene layer's sublayers.
5. The [`GetSolidFilterDefinition`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/buildingscene/arcgisbuildingattributefilter/#solidfilterdefinition) function is used to get the filter definition of the default filter.
6. The [`SetWhereClause`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/buildingscene/arcgissolidbuildingfilterdefinition/#whereclause) function is used to set a dynmically created where clause based on filter criteria.
7. The [`SetActiveBuildingAttributeFilter`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/arcgisbuildingscenelayer/#activebuildingattributefilter) function is used to set the default filter with the new where clause to active.

## Tags
building scene layer, filter, disciplines, categories, construction phases
