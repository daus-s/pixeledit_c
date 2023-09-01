g++ linear_gradient_transparency.cc -o a.out -lpng
./a.out COLUMN.PNG right
mv OUTPUT.PNG ../../../portfolio/public/rightcolumn.png
./a.out COLUMN.PNG left
mv OUTPUT.PNG ../../../portfolio/public/leftcolumn.png
cd ../../../portfolio 
git add public/*column*
git commit -m "changed gradient via cc.sh routine"
git push

