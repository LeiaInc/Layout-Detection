# Change Log

## [0.0.4] - 2021-02-10
 
### Added
- New SVM model that classifies TWO_BY_ONE_HALF image.
 
### Changed
- ImageLayoutType enum was moved in the photoformat lib.
 
### Fixed

## [0.0.3] - 2021-01-29
 
### Added
 
### Changed
 
### Fixed
- The previous model was old version. 
- Updated the svm model file with the correct one.
- About the new model: [The gamma parameter of the svm is set to 1.0](https://github.com/LeiaInc/layoutDetection/blob/86978085525c538a442f4fb6d030038c8d6ac157/training/main.cpp#L149)

## [0.0.2] - 2021-01-25
 
### Added
 
### Changed
  
- [Cv 1675 threshold (#11)](https://leia3d.atlassian.net/browse/CV-1638)
  Set the threshold value for the confidence score and returns UNKNOWN class label if the cofidence score is below the threshold.
 
### Fixed

## [0.0.1] - 2021-01-13
 
### Added
- Initial version uploaded
- Fast lane setup
- Code refactor
 
### Changed
 
### Fixed