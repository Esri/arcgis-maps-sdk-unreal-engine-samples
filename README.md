# arcgis-maps-sdk-unreal-engine-samples

![image](arcgis-maps-sdk-unreal-engine-samples.png)

Here is an Unreal Engine 5 project containing a set of samples showing you how to accomplish various things using the combined features of Unreal Engine 5 and the ArcGIS Maps SDK for Unreal Engine. The `main` branch is configured to work with our most recent release (1.6.0). If you want to use the sample repo with an older release, check out the corresponding tag of the sample repo, `git checkout 1.0.0` for the sample repo that worked with our 1.0.0 release.

### Note

This repository is composed of two separate Unity projects. If you would like to see the samples made for regular use such as the Feature Layer sample and the Routing sample, please use and set up the **sample_project**. If you are interested in XR Samples such as the Virtual Reality Sample and the XR version of our Table Top Sample (coming soon), please use and set up the **xr_sample_project**. Both projects may be used and set up simultaneously, but they do not contain the same samples.

### Requirements for Sample Project

* Computer running Windows or macOS
* The minimum version of Unreal Engine supported for the project is 5.3
* ArcGISMaps SDK for Unreal Engine

### Requirements for XR Sample Project

* Computer running Windows (OpenXR is not supported on macOS)
* The minimum version of Unreal Engine supported for the project is 5.5
* ArcGISMaps SDK for Unreal Engine
* A VR Headset and the necessary software to run through Desktop Mode

## Features in Sample Project

* [Building Filter](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/BuildingFilter) - Explore a building scene layer by toggling the visibility of different attributes.
* [Feature Layer](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/FeatureLayer) - Query objects from a feature layer and show them on the map in Unreal Engine.
* [Geocoding](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/Geocoding) - Search for an address or click on the surface to get the address of that location.
* [HitTest](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/HitTest) - Visualize individual Buildings ID's from a 3D Object Scene Layer.
* [Line of sight](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/LineOfSight) - See how to check the line of sight between two objects.
* [Material By Attribute](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/MaterialByAttribute) - Apply materials to 3DObject Scene layer based on specific attributes.
* [Measure](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/Measure) - Click on the map to get real world distances between points.
* [Routing](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/Routing) - Show a route between two points on a map using Esri's routing service REST API.
* [Stream Layer](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/StreamLayer) - See how to use web sockets to connect to an Esri real time service to update game objects locations in real time.
* [Third Person Character Controller](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/ThirdPersonCharacter) - See how to run around a virtual city in Unreal Engine.
* [Real Time Weather Query](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/sample_project/Content/SampleViewer/Samples/RealTime_Weather) - Query the current weather in a city using the feature layer query work flow.

## Features in XR Sample Project

* [VR Sample](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/xr_sample_project/Content/Samples/VRSample/readme.md) - Explore different cities using Virtual Reality.
* [Tabletop XR Sample](https://github.com/Esri/arcgis-maps-sdk-unreal-engine-samples/tree/main/xr_sample_project/Content/Samples/XRTableTop/readme.md) - Explore the world in tabletop XR.

## Instructions

1. Clone this repo. **Note** On Windows there is a 260 character limit in your file path. The ArcGISMapsSDK for Unreal Engine Plugin is ~160 characters at the longest point. This samples repo by default adds `arcgis-maps-sdk-unreal-engine-samples\sample_project` for a total of ~215 characters. You can use `git clone <git_repo_url> <your_custom_directory_name>` to remove the lengthy `arcgis-maps-sdk-unreal-engine-samples` to give you more freedom for where it can be cloned.
2. Refer to the [ArcGIS Maps SDK for Unreal Engine's documentation on getting started](https://developers.arcgis.com/unreal-engine/get-started/) on how to download `Unreal Engine 5` and the `ArcGIS Maps SDK for Unreal Engine`.
3. Launch the Unreal Engine project. **Note** If you are prompted that there is a version mismatch, the project may need to be re-compiled. You will need to [launch it through Visual Studio](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine).
4. [Install the plugin](https://developers.arcgis.com/unreal-engine/install-and-set-up/add-the-plugin-to-an-existing-project/#install-the-plugin) into this project.
5. Launch the `SampleViewer` level. You will be prompted to enter an [API key](https://developers.arcgis.com/documentation/security-and-authentication/api-key-authentication/) for the samples to work. To avoid entering it each time you run the viewer, go to `Edit -> Project Settings -> Plugins -> ArcGIS Maps SDK` and set the API key for the project. For the detailed steps to create an API key, see [Create and manage an API key tutorials](https://developers.arcgis.com/documentation/security-and-authentication/api-key-authentication/tutorials/create-an-api-key/) in the _Security and authentication guide_. If you don't have the API keys page in your dashboard, upgrade your account to an [ArcGIS Location Platform account](https://location.arcgis.com/sign-up/).

## Requirements

* Refer to the [ArcGIS Maps SDK for Unreal Engine's documentation on system requirements](https://developers.arcgis.com/unreal-engine/system-requirements/)

## Resources

* [ArcGIS Maps SDK for Unreal Engine's documentation](https://developers.arcgis.com/unreal-engine/)
* [Esri Community forum](https://community.esri.com/t5/arcgis-maps-sdks-for-unreal-engine-questions/bd-p/arcgis-maps-sdks-unreal-engine-questions)
* [Obtaining an API key](https://developers.arcgis.com/documentation/security-and-authentication/api-key-authentication/tutorials/create-an-api-key/)
* [Unreal Engine's documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/)

## Issues

Find a bug or want to request a new feature?  Please let us know by submitting an issue.

## Contributing

Esri welcomes contributions from anyone and everyone. Please see our [guidelines for contributing](https://github.com/esri/contributing).

## Licensing

Copyright 2022 - 2024 Esri

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

A copy of the license is available in the repository's [license.txt](license.txt?raw=true) file.
