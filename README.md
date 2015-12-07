# Experiments with Windows WOF

The Windows Overlay Fileystem allows to present data as a file tree. How it works
is that files are represented as reparse points which point to the actual file data
inside some other container. In particular, WIM files can be the source.

WIM is Microsoft's newest filesytem-on-a-file format. The imagex.exe which comes with
the Windows ADK is used to create or manipulate WIM files.

Previously the functionality was exposed as poorly documented IOCTLS and now it is
exposed as poorly documented Winapi calls in <wofapi.h>, hence this project.

Note: most operations below needs an elevated (admin) account.

## Enter Mr WOF

Assume you have:

    c:\test\win\pic.bmp
    c:\test\win\readme.txt

Then:
    <ADK path>\DISM>imagex /compress fast /capture c:\test\wim c:\test\test.wim "anything"

Creates a test.wim which contains the above two files. You can use the same WIM file to capture
different contents, with the first capture set implicitly with ordinal "1". For example:

    <ADK path>\DISM>imagex /dir c:\test\test.wim 1

Lists the first capture set.

imagex can 'mount' a WIM file capture set somewhere in the filesystem:

    <ADK path>\DISM>imagex.exe /mount c:\Test\test.wim 1 c:\Test\mount\v1

Also fsutil (included with Windows) can also do some basic diagnostic operations. Type fsutil wim to
see the command options.

But that is not the fun part. The fun part is to use the WOF api to take over the mounting, in particular
you can mount any file inside a WIM into any other file, regarless of the file name!!

The basic theory of operation is:
1. Point the WOF FS driver to a WIM file to associate with a volume (say C:)
2. Associate a particular file (in that volume) with a file inside a WIM via its SHA1 hash.

This is acomplished in just 2 windows calls, plus one plain CreateFile call.

## The Code

Assuming you have c:\test\mount\v1 directory and the test.win in c:\test:

    Projects\WoF\Debug>wof.exe C:\xx\mount\v1\foo.bmp
      wim id is 0:0
      success overlaying [C:\Test\test.wim] for[C:\xx\mount\v1\foo.bmp]

After that foo.bmp data is magically being decompressed and provided by pic.bmp inside test.wim. Running it
a second time (with v1 changed to v2 in the source):

    Projects\WoF\Debug>wof.exe C:\xx\mount\v2\foo.bmp
      wim id is 0:1
      success overlaying [C:\Test\test.wim] for[C:\xx\mount\v2\foo.bmp]

We can check via fsutil that we have two data sources (with the same WIM) ready for the C volume:
	  
    fsutil wim enumwims C :
      0 {3FFC560B - 7441 - 4FE5 - A8C8 - A72E1EDE51AC} 00000000 C : \Test\test.wim:1
      1 {3FFC560B - 7441 - 4FE5 - A8C8 - A72E1EDE51AC} 00000000 C : \Test\test.wim:1

After I edit v2\foo.bmp with mspaint it seems the connection with the second overlay is lost:

    fsutil wim enumfiles c : 0
        \\.\c:\Test\mount\v1\foo.bmp
        Objects enumerated : 1

    fsutil wim enumfiles c : 1
        Objects enumerated : 0






