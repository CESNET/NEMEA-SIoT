from setuptools import setup, find_packages

setup(
    name='SIoTpot',
    version='0.0.1',
    description="Master's Thesis work - IoT honeypot",
    author='Simon Stefunko',
    author_email='s.stefunko@gmail.com',
    keywords='IoT, honeypot, IoTpot, Iot honeypot, security',
    license='Public Domain',
    url='https://github.com/HalfDeadPie/IoTpot',
    packages=['siotpot'],
    python_requires='~=2.7',
    classifiers=[
        'Intended Audience :: Developers',
        'License :: Public Domain',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Topic :: Software Development :: Libraries',
        'Framework :: Scapy-radio',
        'Environment :: Console',
    ],
    zip_safe=False,
    entry_points={
        'console_scripts': [
            'siotpot = siotpot.__main__',
        ],
    },
    install_requires=['click>=7', 'configparser>=3.7.4', 'crc16==0.1.1', 'tinydb==3.13.0', 'xxhash==1.3.0']

)
