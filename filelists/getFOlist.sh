
filename=fastOfflineProd_${1}.list

rm $filename

find /star/data09/reco/production_19GeV_2019/ReversedFullField/dev/2019/ -maxdepth 3 -name "st*_$1*.MuDst.root" >> $filename
find /star/data10/reco/production_19GeV_2019/ReversedFullField/dev/2019/ -maxdepth 3 -name "st*_$1*.MuDst.root" >> $filename
find /star/data11/reco/production_19GeV_2019/ReversedFullField/dev/2019/ -maxdepth 3 -name "st*_$1*.MuDst.root" >> $filename
find /star/data12/reco/production_19GeV_2019/ReversedFullField/dev/2019/ -maxdepth 3 -name "st*_$1*.MuDst.root" >> $filename

# to combine several lists into one
#cat fastOfflineProd_20066* > fastOfflineProd.list
