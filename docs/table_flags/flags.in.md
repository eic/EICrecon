<div>
    <h2>Flags</h2>
    <input type="text" id="myInput" onkeyup="filterTableRowsByInput('myInput', ['table_flags'])" placeholder="Search for names..">
    <div class="tabs">
        <div class="tab">
            <input type="radio" id="tab1" name="tab-group" checked>
            <label for="tab1" class="tab-title">Dump flags</label> 
            <section class="tab-content">
                <table id="myTable" class="table_flags">
                    <thead>
                    <tr>
                        <th>Flag name</th>
                        <th>Default value</th>
                        <th>Description</th>
                    </tr>
                    <tbody>
                    <!--TABLE1 BEGIN-->
                        <tr>
                            <td>1</td>
                            <td>2</td>
                            <td>3</td>
                        </tr>
                    <!--TABLE1 END-->
                    </tbody>
                </table>
            </section>
        </div> 
        <div class="tab">
            <input type="radio" id="tab2" name="tab-group">
            <label for="tab2" class="tab-title">User flags</label> 
            <section class="tab-content">
                <table id="myTable2" class="table_flags">
                    <thead>
                        <tr>
                            <th>Flag name</th>
                            <th>Default value</th>
                            <th>Description</th>
                        </tr>
                    </thead>
                    <tbody>
                    <!--TABLE2 BEGIN-->
                        <tr>
                            <td>1</td>
                            <td>2</td>
                            <td>3</td>
                        </tr>
                    <!--TABLE2 END-->
                    </tbody>
                </table>
            </section>
        </div>
        <div class="tab">
            <input type="radio" id="tab3" name="tab-group">
            <label for="tab3" class="tab-title">Difference</label> 
            <section class="tab-content">
                <table id="myTable3" class="table_flags"  >
                    <thead>
                    <tr>
                        <th>Flag name</th>
                        <th>User flag value</th>
                        <th>Dump flag value</th>
                    </tr>
                    </thead>
                    <tbody>
                    <!--TABLE3 BEGIN-->
                        <tr>
                            <td>1</td>
                            <td>2</td>
                            <td>3</td>
                        </tr>
                    <!--TABLE3 END-->
                    </tbody>
                </table>
            </section>
        </div> 
        <div class="tab">
            <input type="radio" id="tab4" name="tab-group">
            <label for="tab4" class="tab-title">All flags</label> 
            <section class="tab-content">
                <table id="myTable4" class="table_flags"  >
                    <thead>
                    <tr>
                        <th>Flag name</th>
                        <th>Default value</th>
                        <th>Description</th>
                    </tr>
                    <tbody>
                    <!--TABLE4 BEGIN-->
                        <tr>
                            <td wrap>1</td>
                            <td wrap>2</td>
                            <td wrap>3</td>
                        </tr>
                    <!--TABLE4 END-->
                    </tbody>
                </table>
            </section>
        </div> 
    </div>
</div>
