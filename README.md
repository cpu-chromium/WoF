# Experiments with Windows WOF

The Windows Overlay Fileystem allows to present data as a file tree. How it works
is that files are represented as reparse points which point to the actual file data
inside some other container. In particular, WIM files can be the source.

WIM is Microsoft's newest filesytem-on-a-file format. The imagex.exe which comes with
the Windows ADK is used to create or manipulate WIM files.

Previously the functionality was exposed as poorly documented IOCTLS and now it is
exposed as poorly documented Winapi calls in <wofapi.h>, hence this project.

Note: most operations below needs an elevated (admin) account.

## Example

Assume you have:

    c:\test\win\pic.bmp
    c:\test\win\readme.txt

Then:
    <ADK path>\DISM>imagex /compress fast /capture c:\test\wim c:\test\test.wim "anything"

Creates a test.wim which contains the above two files. You can use the same WIM file to capture
different contents, with the first capture set implicitly with ordinal "1". For example:

    <ADK path>\DISM>imagex /dir c:\test\test.wim 1

Lists the first capture set.

imagex can 'mount' a WIM file snapshot somewhere in the filesystem:

    <ADK path>\DISM>imagex.exe /mount c:\Test\test.wim 1 c:\Test\mount\v1

But that is not the fun part. The fun part is to use the WOF api to take over the mounting, in particular
you can mount any file inside a WIM into any other file, regarless of the file name!!

The basic theory of operation is:
1. Point the WOF FS driver to a WIM file to associate with a volume (say C:)
2. Associate a particular file (in that volume) with a file inside a WIM via its SHA1 hash.

This is acomplished in just 2 windows calls, plus one plain CreateFile call.
