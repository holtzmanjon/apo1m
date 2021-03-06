from __future__ import division, with_statement  # confidence high

import shutil
import warnings

import numpy as np

import pyfits
from pyfits.tests import PyfitsTestCase

from nose.tools import assert_equal, assert_raises, assert_true, assert_false


class TestChecksumFunctions(PyfitsTestCase):

    def setup(self):
        super(TestChecksumFunctions, self).setup()
        self._oldfilters = warnings.filters[:]
        warnings.filterwarnings(
            'error',
             message='Checksum verification failed')
        warnings.filterwarnings(
            'error',
            message='Datasum verification failed')

    def teardown(self):
        super(TestChecksumFunctions, self).teardown()
        warnings.filters = self._oldfilters

    def test_sample_file(self):
        hdul = pyfits.open(self.data('checksum.fits'), checksum=True)
        hdul.close()

    def test_image_create(self):
        n = np.arange(100)
        hdu = pyfits.PrimaryHDU(n)
        hdu.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul.close()

    def test_nonstandard_checksum(self):
        hdu = pyfits.PrimaryHDU(np.arange(10.0 ** 6))
        hdu.writeto(self.temp('tmp.fits'), clobber=True,
                    checksum='nonstandard')
        del hdu
        hdul = pyfits.open(self.temp('tmp.fits'), checksum='nonstandard')

    def test_scaled_data(self):
        hdul = pyfits.open(self.data('scale.fits'))
        orig_data = hdul[0].data.copy()
        hdul[0].scale('int16', 'old')
        hdul.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul1 = pyfits.open(self.temp('tmp.fits'), checksum=True)
        assert_true((hdul1[0].data == orig_data).all())
        hdul.close()
        hdul1.close()

    def test_uint16_data(self):
        hdul = pyfits.open(self.data('o4sp040b0_raw.fits'), uint=True)
        hdul.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul1 = pyfits.open(self.temp('tmp.fits'), uint=True, checksum=True)
        hdul.close()
        hdul1.close()

    def test_groups_hdu_data(self):
        imdata = np.arange(100.0)
        imdata.shape = (10, 1, 1, 2, 5)
        pdata1 = np.arange(10) + 0.1
        pdata2 = 42
        x = pyfits.hdu.groups.GroupData(imdata, parnames=['abc', 'xyz'],
                                        pardata=[pdata1, pdata2], bitpix=-32)
        hdu = pyfits.GroupsHDU(x)
        hdu.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul1 = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul1.close()

    def test_binary_table_data(self):
        a1 = np.array(['NGC1001', 'NGC1002', 'NGC1003'])
        a2 = np.array([11.1, 12.3, 15.2])
        col1 = pyfits.Column(name='target', format='20A', array=a1)
        col2 = pyfits.Column(name='V_mag', format='E', array=a2)
        cols = pyfits.ColDefs([col1, col2])
        tbhdu = pyfits.new_table(cols)
        tbhdu.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul.close()

    def test_variable_length_table_data(self):
        c1 = pyfits.Column(name='var', format='PJ()',
            array=np.array([[45.0, 56], np.array([11, 12, 13])], 'O'))
        c2 = pyfits.Column(name='xyz', format='2I', array=[[11, 3], [12, 4]])
        tbhdu = pyfits.new_table([c1, c2])
        tbhdu.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul.close()

    def test_ascii_table_data(self):
        a1 = np.array(['abc', 'def'])
        r1 = np.array([11.0, 12.0])
        c1 = pyfits.Column(name='abc', format='A3', array=a1)
        c2 = pyfits.Column(name='def', format='E', array=r1, bscale=2.3,
                           bzero=0.6)
        c3 = pyfits.Column(name='t1', format='I', array=[91, 92, 93])
        x = pyfits.ColDefs([c1, c2, c3], tbtype='TableHDU')
        hdu = pyfits.new_table(x, tbtype='TableHDU')
        hdu.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul.close()

    def test_compressed_image_data(self):
        hdul = pyfits.open(self.data('comp.fits'))
        hdul.writeto(self.temp('tmp.fits'), clobber=True, checksum=True)
        hdul1 = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul1.close()
        hdul.close()

    def test_compressed_image_data_int16(self):
        n = np.arange(100, dtype='int16')
        hdu = pyfits.ImageHDU(n)
        comp_hdu = pyfits.CompImageHDU(hdu.data, hdu.header)
        comp_hdu.writeto(self.temp('tmp.fits'), checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul.close()

    def test_compressed_image_data_float32(self):
        n = np.arange(100, dtype='float32')
        comp_hdu = pyfits.CompImageHDU(n)
        comp_hdu.writeto(self.temp('tmp.fits'), checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)
        hdul.close()

    def test_open_with_no_keywords(self):
        hdul=pyfits.open(self.data('arange.fits'), checksum=True)
        hdul.close()

    def test_append(self):
        hdul = pyfits.open(self.data('tb.fits'))
        hdul.writeto(self.temp('tmp.fits'), clobber=True)
        n = np.arange(100)
        pyfits.append(self.temp('tmp.fits'), n, checksum=True)
        hdul.close()
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)
        assert_equal(hdul[0]._checksum, None)
        hdul.close()

    def test_writeto_convenience(self):
        n = np.arange(100)
        pyfits.writeto(self.temp('tmp.fits'), n, clobber=True, checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)

        assert_true(hasattr(hdul[0], '_datasum') and hdul[0]._datasum,
                    msg='Missing DATASUM keyword')

        assert_true(hasattr(hdul[0], '_checksum') and hdul[0]._checksum,
                    msg='Missing CHECKSUM keyword')

        assert_true(hasattr(hdul[0], '_datasum_comment') and
                    hdul[0]._datasum_comment,
                    msg='Missing DATASUM Card comment')

        assert_true(hasattr(hdul[0], '_checksum_comment') and
                    hdul[0]._checksum_comment,
                    msg='Missing CHECKSUM Card comment')

        hdul.close()

    def test_hdu_writeto(self):
        n = np.arange(100, dtype='int16')
        hdu = pyfits.ImageHDU(n)
        hdu.writeto(self.temp('tmp.fits'), checksum=True)
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)

        assert_true(hasattr(hdul[0], '_datasum') and hdul[0]._datasum,
                    msg='Missing DATASUM keyword')

        assert_true(hasattr(hdul[0], '_checksum') and hdul[0]._checksum,
                    msg='Missing CHECKSUM keyword')

        assert_true(hasattr(hdul[0], '_datasum_comment') and
                    hdul[0]._datasum_comment,
                    msg='Missing DATASUM Card comment')

        assert_true(hasattr(hdul[0], '_checksum_comment') and
                    hdul[0]._checksum_comment,
                    msg='Missing CHECKSUM Card comment')

        hdul.close()

    def test_datasum_only(self):
        n = np.arange(100, dtype='int16')
        hdu = pyfits.ImageHDU(n)
        hdu.writeto(self.temp('tmp.fits'), clobber=True, checksum='datasum')
        hdul = pyfits.open(self.temp('tmp.fits'), checksum=True)

        assert_true(hasattr(hdul[0], '_datasum') and hdul[0]._datasum,
                    msg='Missing DATASUM keyword')

        assert_true(hasattr(hdul[0], '_checksum') and not hdul[0]._checksum,
                    msg='CHECKSUM keyword should be blank')

        assert_true(hasattr(hdul[0], '_datasum_comment') and
                    hdul[0]._datasum_comment,
                    msg='Missing DATASUM Card comment')

        assert_true(hasattr(hdul[0], '_checksum_comment') and
                    not hdul[0]._checksum_comment,
                    msg='CHECKSUM Card comment should be blank')

        hdul.close()

    def test_open_update_mode_preserve_checksum(self):
        """
        Regression test for #148 where checksums are being removed from headers
        when a file is opened in update mode, even though no changes were made
        to the file.
        """

        shutil.copy(self.data('checksum.fits'), self.temp('tmp.fits'))

        with pyfits.open(self.temp('tmp.fits')) as hdul:
            data = hdul[1].data.copy()

        hdul = pyfits.open(self.temp('tmp.fits'), mode='update')
        hdul.close()

        with pyfits.open(self.temp('tmp.fits')) as hdul:
            assert_true('CHECKSUM' in hdul[1].header)
            assert_true('DATASUM' in hdul[1].header)
            assert_true((data == hdul[1].data).all())

    def test_open_update_mode_update_checksum(self):
        """
        Regression test for #148, part 2.  This ensures that if a file contains
        a checksum, the checksum is updated when changes are saved to the file,
        even if the file was opened with the default of checksum=False.

        An existing checksum and/or datasum are only stripped if the file is
        opened with checksum='remove'.
        """

        shutil.copy(self.data('checksum.fits'), self.temp('tmp.fits'))
        with pyfits.open(self.temp('tmp.fits')) as hdul:
            header = hdul[1].header.copy()
            data = hdul[1].data.copy()

        with pyfits.open(self.temp('tmp.fits'), mode='update') as hdul:
            hdul[1].header['FOO'] = 'BAR'
            hdul[1].data[0]['TIME'] = 42

        with pyfits.open(self.temp('tmp.fits')) as hdul:
            header2 = hdul[1].header
            data2 = hdul[1].data
            assert_equal(header2[:-3], header[:-2])
            assert_true('CHECKSUM' in header2)
            assert_true('DATASUM' in header2)
            assert_equal(header2['FOO'], 'BAR')
            assert_true((data2['TIME'][1:] == data['TIME'][1:]).all())
            assert_equal(data2['TIME'][0], 42)

        with pyfits.open(self.temp('tmp.fits'), mode='update',
                         checksum='remove') as hdul:
            pass

        with pyfits.open(self.temp('tmp.fits')) as hdul:
            header2 = hdul[1].header
            data2 = hdul[1].data
            assert_equal(header2[:-1], header[:-2])
            assert_false('CHECKSUM' in header2)
            assert_false('DATASUM' in header2)
            assert_equal(header2['FOO'], 'BAR')
            assert_true((data2['TIME'][1:] == data['TIME'][1:]).all())
            assert_equal(data2['TIME'][0], 42)
