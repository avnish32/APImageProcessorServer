# README
This is the server module of the Image Processing Application. This application communicates with the Client module to receive an image and a filter to apply on it, and returns the image with the filter applied.

## Requirements
 - 64-bit Windows
 - OpenCV 4.8.0
 - Client companion module
 - Visual Studio (2022 preferred)

## Configuring OpenCV 4.8.0

1. Download 4.8.0 from [OpenCV Releases page](https://opencv.org/releases/).
2. Install OpenCV to the desired location.
3. Add the OpenCV directory environment variable. Below are the steps to do this on Windows:
  - Open Start menu, search for 'Edit the system environment variables' and click on 'Open' as shown:

       <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/4122d972-7555-4a4b-838e-ea73aa59ac4f" width = "500">

  - In the 'System Properties' dialog box, click the 'Environment Variables...' button:

      <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/63b1ecd0-a510-4b7d-9d29-3a7a3698cd8f" width = "300">

  - Click 'New' at the bottom:
     
    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/6eb42f45-d1e2-4ca7-a112-0e1c035becd4" width = "300">

  - Type in 'OPENCV_DIR' (without the quotes) in the 'Variable name' field as shown, and click on the 'Browse Directory' button:

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/9c0547cf-6da5-49b7-8978-cef459e4c40e" width = "500">

  - Browse to the 'build' folder of your OpenCV installation, as shown below. For example, if you installed OpenCV in the 'Documents' folder, browse to 'C:\Users\abc\Documents\opencv\build\'. Select the 'build' folder and click 'OK':

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/be91ec41-fd1a-483a-853d-bc7b0bd88261" width = "300">

  - The 'Variable value' field in the 'New System Value' dialog box should now contain the address of the OpenCV build folder, as shown below. Click OK:

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/83e1754e-09b3-44f4-980b-52027040e978" width = "500">

  - The OPENCV_DIR variable will now be added to the 'System Variables' list in the 'Environment Variables' dialog box. Verify the value and click 'OK':

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/8ab10cd7-a281-4659-8d5d-04b48dff141e" width = "300">

  - Finally, click OK in the 'System Properties' dialog box:

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/32eca162-82bc-4a26-b2a2-fdc57379cf5d" width = "300">

  - More information on how to set up environment variable for OpenCV can be found [here](https://www.opencv-srf.com/2017/11/install-opencv-with-visual-studio.html).

## Running the server
1. Open the .sln file in Visual Studio. Visual Studio 2022 is recommended. Please follow the below steps for this:
   - Open Visual Studio and click on 'Open a project or solution'.

      <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/25e90b0b-6963-4c09-81b5-c0a156fb7a0f" width = "500">

   - Browse to the 'APImageProcessorServer.sln' file and click 'Open'.

      <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/e61ea9c9-559f-4add-8ea7-696d86688ab1" width = "500">
   
2. Build the application. Press Ctrl + B in Visual Studio for this. Below message will be displayed on a successful build:

   <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/d1afa649-d9ff-4621-bcfe-f7c1577893ef" width = "500">
   
3. The program is ready to run now. Execution starts in APImageProcessorServer.cpp. Press Ctrl + F5 in Visual Studio to run the program.
4. On successful execution, a console window like the one shown below should appear:

   ![image](https://github.com/avnish32/APImageProcessorServer/assets/145987378/d03e7736-042c-468e-9c6e-d7d494aebca4)

5. By default, the server runs on port 8080 on the local machine. Support to change this setting is not available currently.

