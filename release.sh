TARBALL_DIR=../tarballs
UNORDERED_DST=$TARBALL_DIR/unordered

mkdir -p $TARBALL_DIR
rm $TARBALL_DIR/unordered.tar.gz
rm $TARBALL_DIR/unordered.zip
rm -rf $TARBALL_DIR/unordered

svn export . $UNORDERED_DST

cd $UNORDERED_DST/doc
bjam
cd -

mkdir $UNORDERED_DST/doc/html/images/
cp $BOOST_ROOT/doc/html/*.css $UNORDERED_DST/doc/html/
cp $BOOST_ROOT/doc/html/images/*.png $UNORDERED_DST/doc/html/images/

rm -r $UNORDERED_DST/libs/functional
rm -r $UNORDERED_DST/bin.v2
rm $UNORDERED_DST/release.sh

cd $TARBALL_DIR
tar -czf unordered.tar.gz unordered
zip -r unordered.zip unordered
cd -

rm -r $UNORDERED_DST

