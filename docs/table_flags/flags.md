
<style>
    body{
        font-family: Source Sans Pro,Helvetica Neue,Arial,sans-serif;
    }
    .markdown-section table{
        display: inline-table;
    }
    table{
        
        width: 10px;
        border-collapse: collapse;
        margin: 25px 0;
        font-size: 0.9em;
        min-width: 400px;
        box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);
        
    }
    thead th {
        background-color: #009879;
        color: #ffffff;
        text-align: left;
        text-align: center;
    }
    tbody th {
        text-align: left;
        padding: 12px 15px;
    }
    #myInput {
        background-image: url('search.png'); 
        background-position: 10px 12px; 
        background-repeat: no-repeat; 
        width: 100%;
        font-size: 16px; 
        padding: 12px 20px 12px 40px; 
        border: 1px solid #ddd; 
        margin-bottom: 12px; 
}
</style>

<div>
<input type="text" id="myInput" onkeyup="filterTableRowsByInput('myInput', 'myTable')" placeholder="Search for names..">
<table id="myTable">
    <thead>
        <tr>
            <th>Flag name</th>
            <th>Default value</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>BEMC:capacityADC</th>
            <th># 8096 </th>
            <th>'8096' ''</th>
        </tr>
        <tr>
            <th>BEMC\:dimScaledLocalDistXY</th>
            <th># 1.8,1.8 </th>
            <th>'1.8,1.8' ''</th>            
        </tr>
        <tr>
            <th>BEMC\:dynamicRangeADC</th>
            <th># 0.1 </th>
            <th>'0.1' ''</th>                
        </tr>
        <tr>
            <th>BEMC\:EcalBarrelClusters:depthCorrection</th>
            <th># 0 </th>
            <th>'0' ''</th>
        </tr>
        <tr>
            <th>BEMC\:EcalBarrelClusters:enableEtaBounds</th>
            <th># 0</th>
            <th>'0' ''</th>
        </tr>
        <tr>
            <th>BEMC:EcalBarrelClusters:energyWeight</th>
            <th>log</th>
            <th>log</th>
        </tr>
        <tr>
            <th>BEMC:EcalBarrelClusters:input_protoclust_tag</th>
            <th># EcalBarrelIslandProtoClusters </th>
            <th>EcalBarrelIslandProtoClusters</th>
        </tr>
    </tbody>
</table>
</div>