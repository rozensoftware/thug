# Thug

Version: 1.0

The solution presented consists of two projects: Thug Creator and Thug (a viewer).

Thug Creator - allows you to save the selected image, change the icon and create the 
resulting file with the correspondingly changed name.

Thug - is an executable file that is displayed in Windows Explorer as an image, which can fool 
the victim into viewing it.

## Description

The technique shown can be used to infect the target system, such as by sending the resulting 
executable file as an email attachment or by other means. 
It is designed to deceive the victim into viewing the image, resulting in the running of code, 
such as creating a backdoor or other.

## Thug Creator

To create a thug file with your image, you need to run the Thug Creator program and specify the arguments as follows:

```powershell
./thugcreator thug.exe my_nasty_backdoor.exe test.jpg image.ico
```

where:

- thug.exe - the name of the base thug.exe
- my_nasty_backdoor.exe - the name of the resulting file
- test.jpg - the name of the image that will be displayed after running the resulting file
- image.ico - the name of the icon that will be displayed in the Windows Explorer, which should be the same as the image

All files must be in the same directory as the Thug Creator program.

## Operating principle

Creator loads a jpg file into a special section in the header of the exe file. 
The maximum size of the image is 1MB. The file's icon is also changed so that Windows Explorer shows 
the file as a regular photo. 
The resulting file name is also changed so that the extension jpg is displayed, instead of exe.

When the user runs the tagret file, the saved photo will be displayed using the Windows photo viewer or 
another default app registered on the system. 
Then the actual photo will be saved to the location of the file, 
running the malicious code and deleting the executable file. 
Only the correct jpg image file will remain in the directory.

## Antivirus detection

The trick of inserting code (read right to left) into the file name is well known to the 
current version of Windows Defender, which blocks the program. 
No less, there are many versions of Windows that could still be a potential target for attack. 
Be that as it may, the example shown may serve as one of many possible attacks by the security 
expert in his attempts to check the vulnerability of computer systems running under Windows OS.

## Disclaimer

The author of this code is not responsible for the incorrect operation of the presented code and/or for its incorrect use. The code presented in this project is intended to serve only to learn programming.

## License

This project is licensed under MIT license (LICENSE-MIT or <http://opensource.org/licenses/MIT>).
