

Here are some of the commands used to initially set up this
repository with some releavant submodules.

~~~
git clone https://github.com/eic/EICrecon

cd I_O
git clone https://github.com/AIDASoft/podio -b v00-14-03 podio.v00-14-03
git submodule add https://github.com/AIDASoft/podio podio.v00-14-03

git clone https://github.com/key4hep/EDM4hep -b v00-05 EDM4hep.v00-05
git submodule add https://github.com/key4hep/EDM4hep EDM4hep.v00-05

git submodule update --init
~~~

