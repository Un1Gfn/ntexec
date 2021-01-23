
## Workaround for [SU Q 1572456](https://superuser.com/q/1572456/)

Download statically linked binaries from release page

On Windows

```bat
> .\win_server.exe
```

On Linux

```bash
./ntexec.out 'c:\Windows\System32\cmd.exe'

# https://ss64.com/nt/syntax-mmc.html
./ntexec.out 'c:\Windows\System32\mmc.exe'
./ntexec.out 'c:\Windows\System32\services.msc'

./ntexec.out 'c:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
./ntexec.out https://www.github.com

./ntexec.out q
```

<details><summary>h</summary>

ReadFile() WriteFile() ReadFileEx() WriteFileEx()

WSARecv() WSARecvFrom() WSARecvMsg()

WSASend() WSASendMsg() WSASendTo()

WSADuplicateSocket() getsockopt() setsockopt()

https://docs.microsoft.com/en-us/windows/win32/winsock/connection-setup-and-teardown-2
https://docs.microsoft.com/en-us/windows/win32/winsock/graceful-shutdown-linger-options-and-socket-closure-2
WSAAccept() WSAConnect() WSASendDisconnect() WSARecvDisconnect() shutdown() closesocket()

[Set up Windows 10 built-in sshd w/ publickey properly](https://stackoverflow.com/a/50502015/8243991)

RPC
* [Windows MIDL](https://docs.microsoft.com/en-us/windows/win32/midl/midl-start-page) /
* [Wine WIDL](https://sourceforge.net/p/mingw-w64/wiki2/Wine%20integration/)

Mingw-w64
* https://aur.archlinux.org/packages/?K=mingw-w64&PP=250&do_Search=Go
* https://wiki.archlinux.org/index.php/MinGW_package_guidelines
* https://aur.archlinux.org/packages/mingw-w64-allegro/

</details>

![meme](https://i.imgflip.com/4ut4nl.jpg) <!-- https://imgflip.com/i/4ut4nl -->

<details><summary></summary>

---

"gru's plan"

Start server  on   Windows

Send url to Windows from Linux

Chrome opens url on Windows

̶W̶i̶n̶d̶o̶w̶s̶ ̶O̶O̶M̶e̶d̶ ̶ ̶ ̶b̶y̶ ̶ ̶C̶h̶r̶o̶m̶e̶

https://lingojam.com/FancyTextGenerator

https://imgflip.com/memegenerator

---

</details>

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/e5f5064e6dab41d98dc731adf8a09274)](https://www.codacy.com/gh/Un1Gfn-nt/ntexec/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Un1Gfn-nt/ntexec&amp;utm_campaign=Badge_Grade)
