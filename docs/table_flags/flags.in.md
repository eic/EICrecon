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
