imageFileNames = {'D:/study/course/cv/hmw/Image1.jpg',...
    'D:/study/course/cv/hmw/Image2.jpg',...
    'D:/study/course/cv/hmw/Image3.jpg',...
    'D:/study/course/cv/hmw/Image4.jpg'
    };

[imagePoints, boardSize, imagesUsed] = detectCheckerboardPoints(imageFileNames);
imageFileNames = imageFileNames(imagesUsed);

squareSize = 25.6;  % in units of 'mm'
worldPoints = generateCheckerboardPoints(boardSize, squareSize);

cameraParams = estimateCameraParameters(imagePoints, worldPoints, ...
    'EstimateSkew', false, 'EstimateTangentialDistortion', false, ...
    'NumRadialDistortionCoefficients', 2, 'WorldUnits', 'mm');

h1=figure; showReprojectionErrors(cameraParams, 'BarGraph');

h2=figure; showExtrinsics(cameraParams, 'CameraCentric');

originalImage = imread(imageFileNames{1});
undistortedImage = undistortImage(originalImage, cameraParams);

