<script>
    new Vue({
        el: '#example_vue',
    // Options...
        data() {
            return {
                flags: [],
                recoPrefixes: [],
                showFlags: false,
                showDefaultValue: false,
                // showUserValue: true,
                showDescription: true,
                // showDiff: false,
                showRecoOnly: false,
            }
        },
        mounted() {
            this.showRecoOnly = (localStorage.getItem("showRecoOnly") ?? "false")  === "true";
            this.showFlags = (localStorage.getItem("showFlags") ?? "false") === "true";
            this.showDefaultValue = (localStorage.getItem("showDefaultValue") ?? "false") === "true";
            // this.showUserValue = (localStorage.getItem("showUserValue") ?? "true") === "true";
            this.showDescription = (localStorage.getItem("showDescription") ?? "true") === "true";
            // this.showDiff = (localStorage.getItem("showDiff") ?? "false") === "true";
        },
        // watch: {
        //     showDiff(value) {
        //         localStorage.setItem("showDiff", value);
        //         if(value) {
        //             this.showDefaultValue = true;
        //             this.showUserValue = true;
        //             this.showDescription = false;
        //         }
        //     },
        //     showRecoOnly(newValue, oldValue) { localStorage.setItem("showRecoOnly", newValue); },
        //     showFlags(newValue, oldValue) { localStorage.setItem("showFlags", newValue); },
        //     showDefaultValue(newValue, oldValue) { localStorage.setItem("showDefaultValue", newValue); },
        //     showUserValue(newValue, oldValue) { localStorage.setItem("showUserValue", newValue); },
        //     showDescription(newValue, oldValue) { localStorage.setItem("showDescription", newValue); },
        //
        // },

        created() {
            // Read json flags
            fetch('e_craterlake_flags.json')
                .then(response => response.json())
                .then(data => (this.flags = data))
                .catch(err => console.log(err));
            // Define reconstruction flags prefixes
            this.recoPrefixes = [
                "B0ECAL",
                "B0TRK",
                "BEMC",
                "BTOF",
                "BTRK",
                "BVTX",
                "ECTOF",
                "ECTRK",
                "EEMC",
                "FOFFMTRK",
                "HCAL",
                "MPGD",
                "RICH",
                "RPOTS",
                "ZDC",
                "Tracking",
                "Reco",
                "Digi",
                "Calorimetry"
            ];
        },
        computed: {
            filteredFlags() {
                let prefixes = this.recoPrefixes;
                return this.flags.filter(el => {
                    // if(this.showDiff) {
                    //
                    //     if(el[0] === "B0ECAL:B0ECalClusters:depthCorrection") {
                    //         console.log(el[0]);
                    //         console.log(el[1]);
                    //         console.log(el[2]);
                    //     }
                    //
                    //     if(el[1].toUpperCase() === el[2].toUpperCase()) {
                    //         return false;
                    //     }
                    // }

                    if(!this.showRecoOnly) {
                        return true;   // We are not filtering and return everything
                    }

                    for (let prefix of prefixes) {
                        if (el[0].toUpperCase().startsWith(prefix.toUpperCase())) {
                            return true;
                        }
                    }
                    return false;
                });
            },
        },
        filters: {
            cutLongLines: function (value) {
                if(value.length > 50) {
                    return value.slice(0,50) + "...";
                }
                return value;
            }
        }
    });
</script>

<div id="example_vue">
    <div class="radio_btn_name">
        <div>Reconstruction only</div>
        <div>Show default value</div>
        <div>Show description</div>
    </div>
    <div class="radio_btn">
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle1" class="mobileToggle" id="toggle1" v-model="showRecoOnly">
            <label for="toggle1"></label>
        </div>
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle2" class="mobileToggle" id="toggle2" v-model="showDefaultValue">
            <label for="toggle2"></label>
        </div>
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle4" class="mobileToggle" id="toggle4" v-model="showDescription">
            <label for="toggle4"></label>
        </div>
    </div>
    <input type="text" id="myInput" onkeyup="filterTableRowsByInput('myInput', ['table_flags'])" placeholder="Search for flags..">
    <table class="table_flags">
        <thead>
            <tr>
                <th v-if="!showFlags">Flag name</th>
                <th v-if="showDefaultValue">Default value</th>
                <th v-if="showDescription">Description</th>
            </tr>
        </thead>
        <tbody>
            <tr v-for="flag in filteredFlags" v-bind:title="flag[3]">
                <td >{{ flag[0] }}</td>
                <td v-if="showDefaultValue" v-bind:title="flag[1]">{{ flag[1] | cutLongLines }}</td>
                <td v-if="showDescription" v-bind:title="flag[3]">{{ flag[3] }}</td>
            </tr>
        </tbody>
    </table>
</div>
