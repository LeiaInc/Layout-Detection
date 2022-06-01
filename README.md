# layoutDetection

Android library for classifying {Single, 2x1, 1x2, 2x2} image.

## Usage

1. Add this snippet to your build.gradle to fetch from our Maven server:
```groovy
repositories {
    // Configure Gradle to download our library from our repository.
    // NOTE: These leiaprivate credentials should not be shared outside of Leia.
    maven {
        url 'https://leiainc.jfrog.io/leiainc/gradle-dev'

        credentials {
            username "leiaprivate"
            password "102852a"
        }
    }
}
```


2. Declare a dependency on our library:

```groovy
dependencies {   
    // You can find the list of releases at https://github.com/LeiaInc/layoutDetection/releases.
    implementation 'com.leiainc:layoutdetection::[latest.release.here]'
}
```

3. Call the libaray from Jave
```groovy
import com.leiainc.androidsdk.layoutdetection.ImageLayoutType;
import com.leiainc.androidsdk.layoutdetection.ImageLayoutDetector;

ImageLayoutDetector imageLayoutDetector = ImageLayoutDetector.getInstance(context);
imageLayoutDetector.setThresConfidenceScore(0.90f);
ImageLayoutType layout = imageLayoutDetector.detectLayout(bitmap);
```
or see this example [here](https://github.com/LeiaInc/layoutDetection/blob/bafb7a11d516ab0c19e728d3901cc3e669f153c3/app/src/main/java/com/example/layoutdetection/MainActivity.java#L38)

The library outputs a enum class ```ImageLayoutType``` with 5 class labels:
```groovy
public enum ImageLayoutType {
    SINGLE,
    TWO_BY_ONE,
    ONE_BY_TWO,
    TWO_BY_TWO,
    UNKNOWN
}
``` 

When the model's confidence score is below than the predefined threshold (```thresConfidenceScore```), it will return ```UNKNOW``` class.

The default value for ```thresConfidenceScore``` is 0.90 (or can be set by calling ```imageLayoutDetector.setThresConfidenceScore(0.90f);```)
This confidence score threshold value can control the false positive (incorrectly classified) and the false negative (miss) rate. 
According to our experiment with 12k test samples, 0.90 can achieve 0 % false positive and 0.14 % miss rates. 

## Build

To build this android project, 

Install CMake version 3.10.2 or higher (https://cmake.org/install/)

When publishing layoutDetection to artifactory please update the version number in version.properties.