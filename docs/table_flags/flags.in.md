<div>
    <h2>Flags</h2>
    <input type="text" id="myInput" onkeyup="filterTableRowsByInput('myInput', ['table_flags'])" placeholder="Search for names..">
    <div class="tabs">
        <div class="tab">
            <input type="radio" id="tab1" name="tab-group" checked>
            <label for="tab1" class="tab-title">EICrecon flags</label> 
            <section class="tab-content">
                <table id="myTable" class="table_flags">
                    <thead>
                    <tr>
                        <th>Flag name</th>
                        <th>Value</th>
                        <th>Default value</th>
                        <th>IsReco</th>
                        <th>Description</th>
                    </tr>
                    <tbody>
                    <!--TABLE BEGIN-->
                        <tr>
                            <td>1</td>
                            <td>2</td>
                            <td>3</td>
                            <td>4</td>
                            <td>5</td>
                        </tr>
                    <!--TABLE END-->
                    </tbody>
                </table>
            </section>
        </div>
    </div>
</div>
