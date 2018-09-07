pkgname=clolcat
pkgver=v1
pkgrel=1
pkgdesc="Much faster lolcat"
arch=('x86' 'x86_64')
url="https://github.com/IchMageBaume/clolcat"
license=('WTFPL')
conflicts=('python-lolcat' 'lolcat' 'c-lolcat')
source=("$pkgname"::'git+https://github.com/IchMageBaume/clolcat.git')
sha256sums=(SKIP)

build() {
  cd "$srcdir/$pkgname"
  make
}

package() {
  cd "$srcdir/$pkgname"
  make DESTDIR="$pkgdir/" install
}
