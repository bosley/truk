# TRUK

This serves as an idea pad / todo


idea is that ruk will emit c for quite some time. we will base all truk compilation targets 
on directories known as "workspaces" 

these workspaces can contain libraries and binaries

Some command ideas

`./truk --new <workspace-name>`

```

<workspace-name>
    |
    |--- .build/ (where we generate c code to)
    |--- workspace.ini      (where we define the C compiler to use and C compiler options)
    |                        - we need to pull from env common paths on generation if not exist
    |                           print a yellow (libfmt) warning that they need to se tit
    |                         workspace also set name, license
    |-- cmds
    |    |
    |    |---- app
    |           |---main.truk
    |
    |
    |-- kits                (modules/ libraries/ extensions - we call them kits)
         |
         |--- hello_world
                |--- kit.truk
                |--- hello.truk



```

later, we can have kit.truk flag if its a c lib and then define it so we can directly interface with whatever lib it points to then its a c ffi truk kit

(from root)
`./truk build app` note that app is under `cmds` any directory under `cmds` that cointans a `main.truk` shoul be able to be built

then 

`./truk test hello_world`   test cmmand can test any one lib or

`./truk test` to invoke all

and ofc

`./truk test hello_world <other> <other>` so they can define order

from within kit.truk if they have a dependence htey should be able to reference as if they were
in the top level of the workspace so in `other` we could do `kit/hello_world` not `../hello_world`