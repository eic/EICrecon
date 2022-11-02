<div>
    <h2>Flags</h2>
    <input type="text" id="myInput" onkeyup="filterTableRowsByInput('myInput',  ['myTable'])" placeholder="Search for names..">
    <div class="tabs">
        <div class="tab">
            <input type="radio" id="tab1" name="tab-group" checked>
            <label for="tab1" class="tab-title">Dump flags</label> 
            <section class="tab-content">
                <table id="myTable">
                    <thead>
                    <tr>
                        <th>Flag name</th>
                        <th>Default value</th>
                        <th>Description</th>
                    </tr>
                    <tbody>
                    <tr>
                        <td>1</td>
                        <td>2</td>
                        <td>3</td>
                    </tr>
                    <tr>
                        <td>4</td>
                        <td>5</td>
                        <td>6</td>
                    </tr>
                    <tr>
                        <td>7</td>
                        <td>8</td>
                        <td>9</td>
                    </tr>
                    </tbody>
                </table>
            </section>
        </div> 
        <div class="tab">
            <input type="radio" id="tab2" name="tab-group">
            <label for="tab2" class="tab-title">User flags</label> 
            <section class="tab-content">
                <table id="myTable2" >
                    <thead>
                        <tr>
                            <th>Flag name</th>
                            <th>Default value</th>
                            <th>Description</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                        <td>rrrrr</td>
                        <td>eeeerrrr</td>
                        <td>tttttttt</td>
                        </tr>
                    </tbody>
                </table>
            </section>
        </div>
        <div class="tab">
            <input type="radio" id="tab3" name="tab-group">
            <label for="tab3" class="tab-title">Difference</label> 
            <section class="tab-content">
                <table id="myTable3">
                    <thead>
                    <tr>
                        <th>Flag name</th>
                        <th>User flag value</th>
                        <th>Dump flag value</th>
                    </tr>
                    </thead>
                    <tbody>
                    <tr><td>BEMC:EcalBarrelRawHits:capacityADC</td><td>capacityBitsADC=14</td><td>8096</td></tr>
<tr><td>BEMC:EcalBarrelRawHits:dynamicRangeADC</td><td>20*GeV</td><td>0.1</td></tr>
<tr><td>BEMC:EcalBarrelRawHits:energyResolutions</td><td>0.0,0.02,0.0</td><td></td></tr>
<tr><td>BEMC:EcalBarrelRawHits:pedestalMean</td><td>100</td><td>400</td></tr>
<tr><td>BEMC:EcalBarrelRawHits:pedestalSigma</td><td>1</td><td>3.2</td></tr>
<tr><td>BEMC:EcalBarrelRecHits:capacityADC</td><td>capacityBitsADC=14</td><td>8096</td></tr>
<tr><td>BEMC:EcalBarrelRecHits:dynamicRangeADC</td><td>20*GeV</td><td>0.1</td></tr>
<tr><td>BEMC:EcalBarrelRecHits:pedestalMean</td><td>100</td><td>400</td></tr>
<tr><td>BEMC:EcalBarrelRecHits:pedestalSigma</td><td>1</td><td>3.2</td></tr>
<tr><td>BEMC:EcalBarrelRecHits:samplingFraction</td><td>0.10856976476514045</td><td>0.998</td></tr>
<tr><td>BEMC:EcalBarrelRecHits:thresholdFactor</td><td>3</td><td>4</td></tr>
<tr><td>BEMC:EcalBarrelRecHits:thresholdValue</td><td>3</td><td>0</td></tr>
<tr><td>BEMC:EcalBarrelIslandProtoClusters:minClusterCenterEdep</td><td>30*MeV</td><td>0.03</td></tr>
<tr><td>BEMC:EcalBarrelIslandProtoClusters:minClusterHitEdep</td><td>1.0*MeV</td><td>0.001</td></tr>
<tr><td>BEMC:EcalBarrelIslandProtoClusters:sectorDist</td><td>5.0*cm</td><td>5</td></tr>
<tr><td>BEMC:EcalBarrelClusters:enableEtaBounds</td><td>1</td><td>0</td></tr>
<tr><td>BEMC:EcalBarrelClusters:logWeightBase</td><td>6.2</td><td>3.6</td></tr>
<tr><td>BEMC:EcalBarrelClusters:samplingFraction</td><td>0.10856976476514045</td><td>1</td></tr>
<tr><td>BEMC:EcalBarrelTruthClusters:enableEtaBounds</td><td>1</td><td>0</td></tr>
<tr><td>BEMC:EcalBarrelTruthClusters:logWeightBase</td><td>6.2</td><td>3.6</td></tr>
<tr><td>BEMC:EcalBarrelTruthClusters:samplingFraction</td><td>0.10856976476514045</td><td>1</td></tr>
<tr><td>EEMC:EcalEndcapNRawHits:capacityADC</td><td>capacityBitsADC=14</td><td>8096</td></tr>
<tr><td>EEMC:EcalEndcapNRawHits:dynamicRangeADC</td><td>20*GeV</td><td>0.1</td></tr>
<tr><td>EEMC:EcalEndcapNRawHits:energyResolutions</td><td>0.0,0.02,0.0</td><td></td></tr>
<tr><td>EEMC:EcalEndcapNRawHits:pedestalMean</td><td>100</td><td>400</td></tr>
<tr><td>EEMC:EcalEndcapNRawHits:pedestalSigma</td><td>1</td><td>3.2</td></tr>
<tr><td>EEMC:EcalEndcapNRecHits:capacityADC</td><td>capacityBitsADC=14</td><td>8096</td></tr>
<tr><td>EEMC:EcalEndcapNRecHits:dynamicRangeADC</td><td>20*GeV</td><td>0.1</td></tr>
<tr><td>EEMC:EcalEndcapNRecHits:pedestalMean</td><td>100</td><td>400</td></tr>
<tr><td>EEMC:EcalEndcapNRecHits:pedestalSigma</td><td>1</td><td>3.2</td></tr>
<tr><td>EEMC:EcalEndcapNRecHits:thresholdValue</td><td>3</td><td>0</td></tr>
<tr><td>EEMC:EcalEndcapPRawHits:capacityADC</td><td>capacityBitsADC=14</td><td>8096</td></tr>
<tr><td>EEMC:EcalEndcapPRawHits:dynamicRangeADC</td><td>3*GeV</td><td>0.1</td></tr>
<tr><td>EEMC:EcalEndcapPRawHits:energyResolutions</td><td>0.00316,0.0015,0.0</td><td></td></tr>
<tr><td>EEMC:EcalEndcapPRawHits:fieldRefNumbers</td><td>1,1</td><td></td></tr>
<tr><td>EEMC:EcalEndcapPRawHits:pedestalMean</td><td>100</td><td>400</td></tr>
<tr><td>EEMC:EcalEndcapPRawHits:pedestalSigma</td><td>0.7</td><td>3.2</td></tr>
<tr><td>EEMC:EcalEndcapPRecHits:capacityADC</td><td>capacityBitsADC=14</td><td>8096</td></tr>
<tr><td>EEMC:EcalEndcapPRecHits:dynamicRangeADC</td><td>3*GeV</td><td>0.1</td></tr>
<tr><td>EEMC:EcalEndcapPRecHits:pedestalMean</td><td>100</td><td>400</td></tr>
<tr><td>EEMC:EcalEndcapPRecHits:pedestalSigma</td><td>0.7</td><td>3.2</td></tr>
<tr><td>EEMC:EcalEndcapPRecHits:samplingFraction</td><td>0.03</td><td>0.998</td></tr>
<tr><td>EEMC:EcalEndcapPRecHits:thresholdFactor</td><td>5.0</td><td>4</td></tr>
<tr><td>EEMC:EcalEndcapPRecHits:thresholdValue</td><td>2</td><td>0</td></tr>
<tr><td>HCAL:HcalBarrelRawHits:capacityADC</td><td>capacityBitsADC=8</td><td>8096</td></tr>
<tr><td>HCAL:HcalBarrelRawHits:dynamicRangeADC</td><td>20*MeV</td><td>0.1</td></tr>
<tr><td>HCAL:HcalBarrelRawHits:fieldRefNumbers</td><td>1,0</td><td></td></tr>
<tr><td>HCAL:HcalBarrelRawHits:pedestalMean</td><td>20</td><td>400</td></tr>
<tr><td>HCAL:HcalBarrelRawHits:pedestalSigma</td><td>0.3</td><td>3.2</td></tr>
<tr><td>HCAL:HcalBarrelRecHits:capacityADC</td><td>capacityBitsADC=8</td><td>8096</td></tr>
<tr><td>HCAL:HcalBarrelRecHits:dynamicRangeADC</td><td>20*MeV</td><td>0.1</td></tr>
<tr><td>HCAL:HcalBarrelRecHits:pedestalMean</td><td>20</td><td>400</td></tr>
<tr><td>HCAL:HcalBarrelRecHits:pedestalSigma</td><td>0.3</td><td>3.2</td></tr>
<tr><td>HCAL:HcalBarrelRecHits:thresholdValue</td><td>1</td><td>0</td></tr>
<tr><td>HCAL:HcalBarrelClusters:logWeightBase</td><td>6.2</td><td>3.6</td></tr>
<tr><td>HCAL:HcalBarrelIslandProtoClusters:localDistXY</td><td>150,150</td><td></td></tr>
<tr><td>HCAL:HcalBarrelIslandProtoClusters:minClusterHitEdep</td><td>30.0*MeV</td><td>0.0001</td></tr>
<tr><td>HCAL:HcalBarrelIslandProtoClusters:splitCluster</td><td>0</td><td>1</td></tr>
<tr><td>HCAL:HcalBarrelTruthClusters:logWeightBase</td><td>6.2</td><td>3.6</td></tr>
<tr><td>HCAL:HcalEndcapNRawHits:capacityADC</td><td>capacityBitsADC=8</td><td>8096</td></tr>
<tr><td>HCAL:HcalEndcapNRawHits:dynamicRangeADC</td><td>20*MeV</td><td>0.1</td></tr>
<tr><td>HCAL:HcalEndcapNRawHits:pedestalMean</td><td>20</td><td>400</td></tr>
<tr><td>HCAL:HcalEndcapNRawHits:pedestalSigma</td><td>0.3</td><td>3.2</td></tr>
<tr><td>HCAL:HcalEndcapNRecHits:capacityADC</td><td>capacityBitsADC=8</td><td>8096</td></tr>
<tr><td>HCAL:HcalEndcapNRecHits:dynamicRangeADC</td><td>20*MeV</td><td>0.1</td></tr>
<tr><td>HCAL:HcalEndcapNRecHits:pedestalMean</td><td>20</td><td>400</td></tr>
<tr><td>HCAL:HcalEndcapNRecHits:pedestalSigma</td><td>0.3</td><td>3.2</td></tr>
<tr><td>HCAL:HcalEndcapNRecHits:thresholdValue</td><td>1</td><td>0</td></tr>
<tr><td>HCAL:HcalEndcapNIslandProtoClusters:localDistXY</td><td>150,150</td><td></td></tr>
<tr><td>HCAL:HcalEndcapNIslandProtoClusters:minClusterCenterEdep</td><td>30*MeV</td><td>0.03</td></tr>
<tr><td>HCAL:HcalEndcapNTruthClusters:logWeightBase</td><td>6.2</td><td>3.6</td></tr>
<tr><td>HCAL:HcalEndcapPIslandProtoClusters:localDistXY</td><td>150,150</td><td></td></tr>
<tr><td>HCAL:HcalEndcapPIslandProtoClusters:minClusterCenterEdep</td><td>30.0*MeV</td><td>0.03</td></tr>
<tr><td>HCAL:HcalEndcapPClusters:input_protoclust_tag</td><td>HcalEndcapNIslandProtoClusters</td><td>HcalEndcapPIslandProtoClusters</td></tr>
<tr><td>HCAL:HcalEndcapPClusters:input_simhit_tag</td><td>HcalEndcapNHits</td><td>HcalEndcapPHits</td></tr>
<tr><td>HCAL:HcalEndcapPClusters:samplingFraction</td><td>0.025</td><td>1</td></tr>
<tr><td>ZDC:ZDCEcalRecHits:samplingFraction</td><td>1</td><td>0.998</td></tr>
<tr><td>ZDC:ZDCEcalClusters:logWeightBase</td><td>6.2</td><td>3.6</td></tr>
<tr><td>ZDC:ZDCEcalIslandProtoClusters:localDistXY</td><td>50,50</td><td></td></tr>
<tr><td>ZDC:ZDCEcalIslandProtoClusters:minClusterCenterEdep</td><td>3.*MeV</td><td>0.003</td></tr>
<tr><td>ZDC:ZDCEcalIslandProtoClusters:minClusterHitEdep</td><td>0.1*MeV</td><td>0.0001</td></tr>
<tr><td>BTRK:BarrelTrackerHit:TimeResolution</td><td>8</td><td>10</td></tr>
<tr><td>BVTX:BarrelVertexHit:TimeResolution</td><td>8</td><td>10</td></tr>
<tr><td>ECTRK:EndcapTrackerHit:TimeResolution</td><td>8</td><td>10</td></tr>
<tr><td>MPGD:MPGDTrackerHit:TimeResolution</td><td>8</td><td>10</td></tr>

                    </tbody>
                </table>
            </section>
        </div> 
        <div class="tab">
            <input type="radio" id="tab4" name="tab-group">
            <label for="tab4" class="tab-title">All flags</label> 
            <section class="tab-content">
                <table id="myTable4">
                    <thead>
                    <tr>
                        <th>Flag name</th>
                        <th>Default value</th>
                        <th>Description</th>
                    </tr>
                    <tbody>
                        <td>1</td>
                        <td>2</td>
                        <td>3</td>
                    </tbody>
                </table>
            </section>
        </div> 
    </div>
</div>
