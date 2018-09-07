pkgname=clolcat
pkgver=1
pkgrel=1
pkgdesc="Much faster lolcat"
arch=('x86' 'x86_64')
url="https://github.com/IchMageBaume/clolcat"
license=('WTFPL')

# add conflicts

source=("$pkgname"::'git+https://github.com/IchMageBaume/clolcat.git')
sha256sums=(SKIP)

build() {
  cd "$srcdir/$pkgname"
  make
}

package() {
  cd "$srcdir/$pkgname"
  mkdir -p "${pkgdir}/usr/bin"
  make DESTDIR="$pkgdir/usr/bin" install
}
