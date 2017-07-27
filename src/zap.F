      SUBROUTINE ZAPTHATSUCKER(A,ASR,AER,ASC,AEC,B,BSR,BER,BSC,BEC,
     &                         RSIZE,CSIZE,SIG,PERC,TTY,
     &                      NOISEMOD,GAIN,RN)

C  The actual median filtering routine.

      PARAMETER (NMAX=41)
      INTEGER*4 ASR, AER, ASC, AEC, BSR, BER, BSC, BEC
      INTEGER*4 RSIZE, CSIZE, KEY(NMAX**2), ITEMP(NMAX**2)
      INTEGER RHALF, CHALF
      REAL*4 A(ASC:AEC,ASR:AER), B(BSC:BEC,BSR:BER),
     &     HOLD(NMAX*NMAX), HOLD2(NMAX*NMAX)
      LOGICAL TTY, NOISEMOD

      SIGSQ = SIG*SIG
      NFILTER = RSIZE*CSIZE
      IF (NOISEMOD) RNG = RN**2/GAIN**2

C  Center the filter at each pixel and calculate the median for
C  that filter position.  Hold this value in a temporary buffer
C  until it can be packed back into the image.

C  # of replaced pixels.
      NREP   = 0
C  Half-sizes of the filter
      RHALF   = RSIZE/2
      CHALF   = CSIZE/2
C  Middle element.
      MIDDLE  = (NFILTER+1)/2
      DO 8702 I = 1, NFILTER
         KEY(I) = I
 8702 CONTINUE

      ITAKE = MIDDLE
      ITAKE = NINT(NFILTER*PERC+1.E-6)

      DO 8703 J = BSR, BER

C  Initialize the HOLD(*) array.

         K = 0
         DO 8704 II = -CHALF, CHALF
            DO 8705 JJ = -RHALF, RHALF
               K = K + 1
               HOLD(K) = A(BSC+II,J+JJ)
               HOLD2(K) = HOLD(K)
 8705       CONTINUE
 8704    CONTINUE
C  HOLD vector index.
         INDEX = 1

         DO 8706 I = BSC, BEC

C  Calculate median of hold array.  Locate middle element.  Swap
C  everything bigger on left with everything smaller on right end
C  of hold array.  Based on a program by Mike Fich, Berkeley

C  If we are calculating the median, use Fich`s routine.
C  If we are calculating some arbitrary percentile, we
C  need to do a full sort
            IF (ITAKE .EQ. MIDDLE) THEN
C  Left search limit
               ILEFT = 1
C  Right search limit
               IRIGHT = NFILTER
 8707          IF (ILEFT .LT. IRIGHT) THEN
                  ID1 = ILEFT
                  ID2 = IRIGHT
                  XM  = HOLD(KEY(MIDDLE))

C  Locate an element on the left >= the middle element

 20               IF (HOLD(KEY(ID1)) .LT. XM) THEN
                     ID1 = ID1 + 1
                     GO TO 20
                  END IF

C  Locate an element on the right <= middle element

 8709             IF (HOLD(KEY(ID2)) .GT. XM) THEN
                     ID2 = ID2 - 1
                     GO TO 8709
                  END IF

C  Switch sides of keys and increment to find next set.
                  IF (ID1 .LE. ID2) THEN
                     KTEM     = KEY(ID1)
                     KEY(ID1) = KEY(ID2)
                     KEY(ID2) = KTEM
                     ID1      = ID1 + 1
                     ID2      = ID2 - 1
                     IF (ID1 .LE. ID2) GO TO 20
                  END IF

C  Define new limits
                  IF (ID2 .LT. MIDDLE) ILEFT = ID1
                  IF (MIDDLE .LT. ID1) IRIGHT = ID2
                  GO TO 8707
               END IF
            ELSE
               CALL QUICK(HOLD,NFILTER,ITEMP)
            END IF

            AMED = HOLD(KEY(ITAKE))
C  AMED=median

C  Calculate deviation of neighboring pixels with respect to the median

            IF (SIG .EQ. 0.) THEN
               B(I,J) = AMED
               NREP  = NREP+1

            ELSE
C  Calculate variance
               VARIANCE = (A(I,J)-AMED)**2
               IF (NOISEMOD) THEN
                 SUM = AMED/GAIN + RNG
               ELSE
                 SUM      = -VARIANCE
                 DO 8710 K = 1, NFILTER
                    DIFF = HOLD(K)-AMED
C                   Prevent overflow
                    IF(ABS(DIFF) .GT. 1.0E10) THEN
                       DIFF = 1.0E10
                    END IF
                    SUM = SUM + DIFF**2
 8710            CONTINUE
                 SUM = SUM/FLOAT(NFILTER-1)
               END IF
               IF (VARIANCE .GT. SIGSQ*SUM) THEN
C  Replace with median value
                  B(I,J) = AMED
                  NREP  = NREP+1
C                  IF (TTY) THEN
C                     WRITE(olu,100,ERR=9999)
C     &                    J, I, A(I,J), AMED
C                  END IF
 100              FORMAT(1H ,I5,I6,T17,1PE10.3,T30,1PE10.3)
               ELSE
C  Don`t replace
                  B(I,J) = A(I,J)
               END IF
            END IF

C  Load the next column into HOLD.

            DO 8711 JJ = -RHALF, RHALF
               IF (ITAKE .EQ. MIDDLE) THEN
                  K = INDEX + JJ + RHALF
                  HOLD(K) = A(I+CHALF+1,J+JJ)
               ELSE
                  HOLD2(JJ+RHALF+1) =
     &                 A(I+CHALF+1,J+JJ)
               END IF
 8711       CONTINUE

            IF (ITAKE .EQ. MIDDLE) THEN
               IF (INDEX+RSIZE .GT. NFILTER) THEN
                  INDEX = 1
               ELSE
                  INDEX = INDEX + RSIZE
               END IF
            ELSE
               DO 8812 JJ = 1, RSIZE*CSIZE
                  HOLD(JJ) = HOLD2(JJ)
 8812          CONTINUE
            END IF
 8706    CONTINUE
 8703 CONTINUE

C  Replace A(*,*) with the filtered data.

         DO 8712 J = BSR, BER
            DO 8713 I = BSC, BEC
               A(I,J) = B(I,J)
 8713       CONTINUE
 8712    CONTINUE

      PRINT 101, NREP
 101  FORMAT (' Number of pixels zapped was: ',I6)

      RETURN

C  Error during write - issue message
 9999 PRINT *,'FILTERING NOT COMPLETED'
      RETURN
      END
