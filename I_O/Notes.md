

Here are some of the commands used to initially set up this
repository with some releavant submodules.

~~~
git clone https://github.com/eic/EICrecon

cd I_O
git submodule add https://github.com/AIDASoft/podio
cd podio
git checkout v00-14-03


git submodule add https://github.com/key4hep/EDM4hep
cd EDM4hep
git checkout v00-05

git submodule update --init
~~~

