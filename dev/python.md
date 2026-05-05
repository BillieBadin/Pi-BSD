# Python

## `virtualenv` over `venv`

The `venv` module isn't available in FreeBSD, instead use `virtualenv`

As root:
```sh
pkg install python3 py311-virtualenv
```

As your normal user
```sh
virtualenv --system-site-packages venv
. venv/bin/activate
pip install --upgrade pip
```
