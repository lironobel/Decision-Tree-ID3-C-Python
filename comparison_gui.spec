# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['pycore\\comparison_gui.py'],
    pathex=[],
    binaries=[('C:\\Users\\liron\\AppData\\Local\\Programs\\Python\\Python312\\DLLs\\tcl86t.dll', '.'), ('C:\\Users\\liron\\AppData\\Local\\Programs\\Python\\Python312\\DLLs\\tk86t.dll', '.')],
    datas=[('build/decision_tree.exe', 'build'), ('data/iris.csv', 'data'), ('.pack/graphviz_bin', 'graphviz/bin')],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='comparison_gui',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
