# Nuance SDK in Go
[![GoDoc](https://godoc.org/github.com/nuveo/nuance?status.png)](https://godoc.org/github.com/nuveo/nuance)

This is a wrap of OCR libraries provided by Nuance in their SDK.

The goal is to use the OCR SDK features without polluting the code written in Go with many C calls as well provide a more task-based API, therefore this package does not exposes all nuance C API, instead many calls in C will be condensed in a few Go calls.

## Examples

### SetLicence and initialise

```go
n := nuveo.New()
err := n.SetLicense("licence.lcxz", "OEM_CODE")
if err != nil {
    error.Fatal("SetLicense failed:", err)
}

err = n.Init("YOUR_COMPANY", "YOUR_PRODUCT")
if err != nil {
    error.Fatal("Init failed:", err)
}

n.Quit()
n.Free()
```
---
### Processes zones

Processes image and template and extract information from zones.

```go
n := nuveo.New()

// load the license
err := n.SetLicense("licence.lcxz", "OEM_CODE")
if err != nil {
    fmt.Fatal("SetLicense failed:", err)
}

// initialize the library
err = n.Init("YOUR_COMPANY", "YOUR_PRODUCT")
if err != nil {
    fmt.Fatal("Init failed:", err)
}

// load the template file.
err = n.LoadFormTemplateLibrary("template.ftl")
if err != nil {
    fmt.Fatal("LoadFormTemplateLibrary failed:", err)
}

// loads and processes the image then returns a
// map with the zone marked in the templete.
var ret map[string]string
ret, err = n.OCRImgWithTemplate("sample.tif")
if err != nil {
    fmt.Fatal("OCRImgWithTemplate failed:", err)
}

// echoes the result on the screen
for k, v := range ret {
    fmt.Println("k:", k, "v:", v)
}

// close the library and frees the memory
n.Quit()
n.Free()
```

## Running the tests

At this point of development to run unit tests is necessary to provide your own license files, templates and samples.

## Installation

First install the 19.2 version of Nuance SDK

*Debian Linux example:*
```
dpkg -i nuance-omnipage-csdk-lib64_19.2-15521.100_amd64.deb
dpkg -i nuance-omnipage-csdk-devel_19.2-15521.100_amd64.deb
```

Then download the package

```
go get github.com/nuveo/nuance
```

To activate your licenses use the following command

```
oplicmgr -c OEM_CODE -N licence.lcxz PRODUCT_KEY1 PRODUCT_KEY2 PRODUCT_KEY3
```

You will need your license file and your OEM code in all the examples and tests.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

This project is a wrap to Go of libraries provided in the Nuance SDK
