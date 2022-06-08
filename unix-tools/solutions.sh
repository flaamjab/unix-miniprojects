# Вариант 4

# I. grep
grep -Ec '\?|\!' text.txt # 1
grep '^«' text.txt # 2
cat text.txt | sort # 3 (зачем тут grep?)
grep -c Маш text.txt # 4

# II. sed
sed '/.*«.*/c A' text.txt # 1
sed 's/.*Маша.*/Маша/' text.txt # 2
sed 'y/АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя/ЯАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮяабвгдеёжзийклмнопрстуфхцчшщъыьэю/' text.txt # 3
sed '/!$/a\!!!' text.txt # 4

# III.awk

# 1
awk 'FNR == 1 { split(FILENAME, s, "_"); print s[1] }' *10.10.2016

# 2
awk -F , '
  BEGIN { total = 0 }
  {total += $2 * $3} 
  END {
    split(FILENAME, s, "_")
    shop = s[1]
    date = s[2]
    print "The total price of goods bought on " date " is " total "."
}' ЦУМ_10.10.2016

# 3
awk 'FNR == 0 { split(FILENAME, s, "_"); print s[2] }' ПЯТЁРОЧКА*

# 4
awk -F , '
  BEGIN { n_shops = 0 }
  { split(FILENAME, s, "_"); shop = s[1] }
  $1 == "Макароны" { 
    if (prev_shop != shop) {
      n_shops +=1
      prev_shop = shop
    }
  }
  END { print "Макароны покупались в " n_shops " магазинах" }
' *

